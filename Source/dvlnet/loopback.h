#pragma once

#include <cstdint>
#include <queue>
#include <string>

#include "dvlnet/abstract_net.h"

namespace devilution::net {

class loopback : public abstract_net {
private:
	std::queue<buffer_t> message_queue;
	buffer_t message_last;
	uint8_t plr_single = 0;

public:
	loopback() = default;

	int create(std::string addrstr) override;
	int join(std::string addrstr) override;
	bool SNetReceiveMessage(uint8_t *sender, void **data, uint32_t *size) override;
	bool SNetSendMessage(int dest, void *data, unsigned int size) override;
	bool SNetReceiveTurns(char **data, size_t *size, uint32_t *status) override;
	bool SNetSendTurn(char *data, unsigned int size) override;
	void SNetGetProviderCaps(struct _SNETCAPS *caps) override;
	bool SNetRegisterEventHandler(event_type evtype, SEVTHANDLER func) override;
	bool SNetUnregisterEventHandler(event_type evtype) override;
	bool SNetLeaveGame(int type) override;
	bool SNetDropPlayer(int playerid, uint32_t flags) override;
	bool SNetGetOwnerTurnsWaiting(uint32_t *turns) override;
	bool SNetGetTurnsInTransit(uint32_t *turns) override;
	void setup_gameinfo(buffer_t info) override;
	std::string make_default_gamename() override;
};

} // namespace devilution::net
