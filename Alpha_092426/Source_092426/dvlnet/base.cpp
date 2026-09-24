#include "dvlnet/base.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>

#include "player.h"

namespace devilution {
namespace net {

namespace {

void BaseArrayDiag(const char *where, int index, int self, int source = -1, int newPlayer = -1)
{
	char buffer[256];
	std::snprintf(buffer, sizeof(buffer),
	    "[BASE ARRAY] %s index=%d plr_self=%d source=%d newPlayer=%d MAX_PLRS=%d",
	    where, index, self, source, newPlayer, MAX_PLRS);

	std::fprintf(stderr, "%s\n", buffer);
	std::fflush(stderr);

	if (FILE *file = std::fopen("zerotier_diagnostic.txt", "a")) {
		std::fprintf(file, "%s\n", buffer);
		std::fclose(file);
	}
}

bool BaseArrayIndexValid(const char *where, int index, int self, int source = -1, int newPlayer = -1)
{
	BaseArrayDiag(where, index, self, source, newPlayer);
	if (index >= 0 && index < MAX_PLRS)
		return true;

	char buffer[256];
	std::snprintf(buffer, sizeof(buffer),
	    "[BASE ARRAY] ERROR: %s attempted playerStateTable_[%d]; access rejected",
	    where, index);
	std::fprintf(stderr, "%s\n", buffer);
	std::fflush(stderr);
	if (FILE *file = std::fopen("zerotier_diagnostic.txt", "a")) {
		std::fprintf(file, "%s\n", buffer);
		std::fclose(file);
	}
	return false;
}

} // namespace

void base::setup_gameinfo(buffer_t info)
{
	game_init_info = std::move(info);
}

void base::setup_password(std::string pw)
{
	pktfty = std::make_unique<packet_factory>(pw);
}

void base::clear_password()
{
	pktfty = std::make_unique<packet_factory>();
}

void base::RunEventHandler(_SNETEVENT &ev)
{
	auto f = registered_handlers[static_cast<event_type>(ev.eventid)];
	if (f != nullptr) {
		f(&ev);
	}
}

void base::DisconnectNet(plr_t plr)
{
}

void base::SendEchoRequest(plr_t player)
{
	if (plr_self == PLR_BROADCAST)
		return;
	if (player == plr_self)
		return;

	timestamp_t now = SDL_GetTicks();
	auto echo = pktfty->make_packet<PT_ECHO_REQUEST>(plr_self, player, now);
	send(*echo);
}

void base::HandleAccept(packet &pkt)
{
	if (plr_self != PLR_BROADCAST) {
		return; // already have player id
	}
	if (pkt.Cookie() == cookie_self) {
		plr_self = pkt.NewPlayer();
		Connect(plr_self);
	}
	if (game_init_info != pkt.Info()) {
		if (pkt.Info().size() != sizeof(GameData)) {
			ABORT();
		}
		// we joined and did not create
		game_init_info = pkt.Info();
		_SNETEVENT ev;
		ev.eventid = EVENT_TYPE_PLAYER_CREATE_GAME;
		ev.playerid = plr_self;
		ev.data = const_cast<unsigned char *>(pkt.Info().data());
		ev.databytes = pkt.Info().size();
		RunEventHandler(ev);
	}
}

void base::HandleConnect(packet &pkt)
{
	plr_t newPlayer = pkt.NewPlayer();
	Connect(newPlayer);
}

void base::HandleTurn(packet &pkt)
{
	plr_t src = pkt.Source();
	if (!BaseArrayIndexValid("HandleTurn", static_cast<int>(src), static_cast<int>(plr_self), static_cast<int>(pkt.Source())))
		return;
	PlayerState &playerState = playerStateTable_[src];
	std::deque<turn_t> &turnQueue = playerState.turnQueue;
	const turn_t &turn = pkt.Turn();
	turnQueue.push_back(turn);
	MakeReady(turn.SequenceNumber);
}

void base::HandleDisconnect(packet &pkt)
{
	plr_t newPlayer = pkt.NewPlayer();
	if (newPlayer != plr_self) {
		if (IsConnected(newPlayer)) {
			auto leaveinfo = pkt.LeaveInfo();
			_SNETEVENT ev;
			ev.eventid = EVENT_TYPE_PLAYER_LEAVE_GAME;
			ev.playerid = pkt.NewPlayer();
			ev.data = reinterpret_cast<unsigned char *>(&leaveinfo);
			ev.databytes = sizeof(leaveinfo_t);
			RunEventHandler(ev);
			DisconnectNet(newPlayer);
			ClearMsg(newPlayer);
			if (!BaseArrayIndexValid("HandleDisconnect", static_cast<int>(newPlayer), static_cast<int>(plr_self), static_cast<int>(pkt.Source()), static_cast<int>(newPlayer)))
				return;
			PlayerState &playerState = playerStateTable_[newPlayer];
			playerState.isConnected = false;
			playerState.turnQueue.clear();
		}
	} else {
		ABORT(); // we were dropped by the owner?!?
	}
}

void base::HandleEchoRequest(packet &pkt)
{
	auto reply = pktfty->make_packet<PT_ECHO_REPLY>(plr_self, pkt.Source(), pkt.Time());
	send(*reply);
}

void base::HandleEchoReply(packet &pkt)
{
	uint32_t now = SDL_GetTicks();
	plr_t src = pkt.Source();
	if (!BaseArrayIndexValid("HandleEchoReply", static_cast<int>(src), static_cast<int>(plr_self), static_cast<int>(pkt.Source())))
		return;
	PlayerState &playerState = playerStateTable_[src];
	playerState.roundTripLatency = now - pkt.Time();
}

void base::ClearMsg(plr_t plr)
{
	message_queue.erase(std::remove_if(message_queue.begin(),
	                        message_queue.end(),
	                        [&](message_t &msg) {
		                        return msg.sender == plr;
	                        }),
	    message_queue.end());
}

void base::Connect(plr_t player)
{
	if (!BaseArrayIndexValid("Connect", static_cast<int>(player), static_cast<int>(plr_self)))
		return;
	PlayerState &playerState = playerStateTable_[player];
	bool wasConnected = playerState.isConnected;
	playerState.isConnected = true;

	if (!wasConnected)
		SendFirstTurnIfReady(player);
}

bool base::IsConnected(plr_t player) const
{
	if (!BaseArrayIndexValid("IsConnected", static_cast<int>(player), static_cast<int>(plr_self)))
		return false;
	const PlayerState &playerState = playerStateTable_[player];
	return playerState.isConnected;
}

void base::RecvLocal(packet &pkt)
{
	if (pkt.Source() < MAX_PLRS) {
		Connect(pkt.Source());
	}
	switch (pkt.Type()) {
	case PT_MESSAGE:
		message_queue.emplace_back(pkt.Source(), pkt.Message());
		break;
	case PT_TURN:
		HandleTurn(pkt);
		break;
	case PT_JOIN_ACCEPT:
		HandleAccept(pkt);
		break;
	case PT_CONNECT:
		HandleConnect(pkt);
		break;
	case PT_DISCONNECT:
		HandleDisconnect(pkt);
		break;
	case PT_ECHO_REQUEST:
		HandleEchoRequest(pkt);
		break;
	case PT_ECHO_REPLY:
		HandleEchoReply(pkt);
		break;
	default:
		break;
		// otherwise drop
	}
}

bool base::SNetReceiveMessage(uint8_t *sender, void **data, uint32_t *size)
{
	poll();
	if (message_queue.empty())
		return false;
	message_last = message_queue.front();
	message_queue.pop_front();
	*sender = message_last.sender;
	*size = message_last.payload.size();
	*data = message_last.payload.data();
	return true;
}

bool base::SNetSendMessage(int playerId, void *data, unsigned int size)
{
	if (playerId != SNPLAYER_ALL && playerId != SNPLAYER_OTHERS
	    && (playerId < 0 || playerId >= MAX_PLRS))
		abort();
	auto *rawMessage = reinterpret_cast<unsigned char *>(data);
	buffer_t message(rawMessage, rawMessage + size);
	if (playerId == plr_self || playerId == SNPLAYER_ALL)
		message_queue.emplace_back(plr_self, message);
	plr_t dest;
	if (playerId == SNPLAYER_ALL || playerId == SNPLAYER_OTHERS)
		dest = PLR_BROADCAST;
	else
		dest = playerId;
	if (dest != plr_self) {
		auto pkt = pktfty->make_packet<PT_MESSAGE>(plr_self, dest, message);
		send(*pkt);
	}
	return true;
}

bool base::AllTurnsArrived()
{
	for (size_t i = 0; i < Players.size(); ++i) {
		if (!BaseArrayIndexValid("AllTurnsArrived loop", static_cast<int>(i), static_cast<int>(plr_self)))
			return false;
		PlayerState &playerState = playerStateTable_[i];
		if (!playerState.isConnected)
			continue;

		std::deque<turn_t> &turnQueue = playerState.turnQueue;
		if (turnQueue.empty()) {
			LogDebug("Turn missing from player {}", i);
			return false;
		}
	}

	return true;
}

bool base::SNetReceiveTurns(char **data, size_t *size, uint32_t *status)
{
	poll();

	for (size_t i = 0; i < Players.size(); ++i) {
		status[i] = 0;

		if (!BaseArrayIndexValid("SNetReceiveTurns status loop", static_cast<int>(i), static_cast<int>(plr_self)))
			return false;
		PlayerState &playerState = playerStateTable_[i];
		if (!playerState.isConnected)
			continue;

		status[i] |= PS_CONNECTED;

		std::deque<turn_t> &turnQueue = playerState.turnQueue;
		while (!turnQueue.empty()) {
			const turn_t &turn = turnQueue.front();
			seq_t diff = turn.SequenceNumber - current_turn;
			if (diff <= 0x7F)
				break;
			turnQueue.pop_front();
		}
	}

	if (AllTurnsArrived()) {
		for (size_t i = 0; i < Players.size(); ++i) {
			if (!BaseArrayIndexValid("SNetReceiveTurns arrived loop", static_cast<int>(i), static_cast<int>(plr_self)))
				return false;
			PlayerState &playerState = playerStateTable_[i];
			if (!playerState.isConnected)
				continue;

			std::deque<turn_t> &turnQueue = playerState.turnQueue;
			if (turnQueue.empty())
				continue;

			const turn_t &turn = turnQueue.front();
			if (turn.SequenceNumber != current_turn)
				continue;

			playerState.lastTurnValue = turn.Value;
			turnQueue.pop_front();

			status[i] |= PS_ACTIVE;
			status[i] |= PS_TURN_ARRIVED;
			size[i] = sizeof(int32_t);
			data[i] = reinterpret_cast<char *>(&playerState.lastTurnValue);
		}

		current_turn++;

		return true;
	}

	for (size_t i = 0; i < Players.size(); ++i) {
		if (!BaseArrayIndexValid("SNetReceiveTurns active loop", static_cast<int>(i), static_cast<int>(plr_self)))
			return false;
		PlayerState &playerState = playerStateTable_[i];
		if (!playerState.isConnected)
			continue;

		std::deque<turn_t> &turnQueue = playerState.turnQueue;
		if (turnQueue.empty())
			continue;

		status[i] |= PS_ACTIVE;
	}

	return false;
}

bool base::SNetSendTurn(char *data, unsigned int size)
{
	if (size != sizeof(int32_t))
		ABORT();

	turn_t turn;
	turn.SequenceNumber = next_turn;
	std::memcpy(&turn.Value, data, size);
	next_turn++;

	if (!BaseArrayIndexValid("SNetSendTurn", static_cast<int>(plr_self), static_cast<int>(plr_self)))
		return false;
	PlayerState &playerState = playerStateTable_[plr_self];
	std::deque<turn_t> &turnQueue = playerState.turnQueue;
	turnQueue.push_back(turn);
	SendTurnIfReady(turn);
	return true;
}

void base::SendTurnIfReady(turn_t turn)
{
	if (awaitingSequenceNumber_)
		awaitingSequenceNumber_ = !IsGameHost();

	if (!awaitingSequenceNumber_) {
		auto pkt = pktfty->make_packet<PT_TURN>(plr_self, PLR_BROADCAST, turn);
		send(*pkt);
	}
}

void base::SendFirstTurnIfReady(plr_t player)
{
	if (awaitingSequenceNumber_)
		return;

	if (!BaseArrayIndexValid("SendFirstTurnIfReady", static_cast<int>(plr_self), static_cast<int>(plr_self)))
		return;
	PlayerState &playerState = playerStateTable_[plr_self];
	std::deque<turn_t> &turnQueue = playerState.turnQueue;
	if (turnQueue.empty())
		return;

	for (turn_t turn : turnQueue) {
		auto pkt = pktfty->make_packet<PT_TURN>(plr_self, player, turn);
		send(*pkt);
	}
}

void base::MakeReady(seq_t sequenceNumber)
{
	if (!awaitingSequenceNumber_)
		return;

	current_turn = sequenceNumber;
	next_turn = sequenceNumber;
	awaitingSequenceNumber_ = false;

	if (!BaseArrayIndexValid("MakeReady", static_cast<int>(plr_self), static_cast<int>(plr_self)))
		return;
	PlayerState &playerState = playerStateTable_[plr_self];
	std::deque<turn_t> &turnQueue = playerState.turnQueue;
	for (turn_t &turn : turnQueue) {
		turn.SequenceNumber = next_turn;
		next_turn++;
		SendTurnIfReady(turn);
	}
}

void base::SNetGetProviderCaps(struct _SNETCAPS *caps)
{
	caps->size = 0;                  // engine writes only ?!?
	caps->flags = 0;                 // unused
	caps->maxmessagesize = 512;      // capped to 512; underflow if < 24
	caps->maxqueuesize = 0;          // unused
	caps->maxplayers = MAX_PLRS;     // capped to 4
	caps->bytessec = 1000000;        // ?
	caps->latencyms = 0;             // unused
	caps->defaultturnssec = 10;      // ?
	caps->defaultturnsintransit = 2; // maximum acceptable number
	                                 // of turns in queue?
}

bool base::SNetUnregisterEventHandler(event_type evtype)
{
	registered_handlers.erase(evtype);
	return true;
}

bool base::SNetRegisterEventHandler(event_type evtype, SEVTHANDLER func)
{
	/*
  engine registers handler for:
  EVENT_TYPE_PLAYER_LEAVE_GAME
  EVENT_TYPE_PLAYER_CREATE_GAME (should be raised during SNetCreateGame
  for non-creating player)
  EVENT_TYPE_PLAYER_MESSAGE (for bnet? not implemented)
  (engine uses same function for all three)
*/
	registered_handlers[evtype] = func;
	return true;
}

bool base::SNetLeaveGame(int type)
{
	auto pkt = pktfty->make_packet<PT_DISCONNECT>(plr_self, PLR_BROADCAST,
	    plr_self, type);
	send(*pkt);
	plr_self = PLR_BROADCAST;
	return true;
}

bool base::SNetDropPlayer(int playerid, uint32_t flags)
{
	auto pkt = pktfty->make_packet<PT_DISCONNECT>(plr_self,
	    PLR_BROADCAST,
	    (plr_t)playerid,
	    (leaveinfo_t)flags);
	send(*pkt);
	RecvLocal(*pkt);
	return true;
}

plr_t base::GetOwner()
{
	if (Players.size() > MAX_PLRS)
		BaseArrayDiag("GetOwner Players.size exceeds MAX_PLRS", static_cast<int>(Players.size()), static_cast<int>(plr_self));
	for (size_t i = 0; i < Players.size(); ++i) {
		if (IsConnected(i)) {
			return i;
		}
	}
	return PLR_BROADCAST; // should be unreachable
}

bool base::SNetGetOwnerTurnsWaiting(uint32_t *turns)
{
	poll();

	plr_t owner = GetOwner();
	if (!BaseArrayIndexValid("SNetGetOwnerTurnsWaiting", static_cast<int>(owner), static_cast<int>(plr_self)))
		return false;
	PlayerState &playerState = playerStateTable_[owner];
	std::deque<turn_t> &turnQueue = playerState.turnQueue;
	*turns = turnQueue.size();

	return true;
}

bool base::SNetGetTurnsInTransit(uint32_t *turns)
{
	if (plr_self == PLR_BROADCAST || plr_self >= MAX_PLRS) {
		BaseArrayDiag("SNetGetTurnsInTransit no local player; reporting zero turns",
		    static_cast<int>(plr_self), static_cast<int>(plr_self));
		*turns = 0;
		return true;
	}

	BaseArrayDiag("SNetGetTurnsInTransit",
	    static_cast<int>(plr_self), static_cast<int>(plr_self));
	PlayerState &playerState = playerStateTable_[plr_self];
	std::deque<turn_t> &turnQueue = playerState.turnQueue;
	*turns = turnQueue.size();
	return true;
}

} // namespace net
} // namespace devilution
