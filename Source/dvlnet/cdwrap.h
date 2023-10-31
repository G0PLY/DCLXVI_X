#pragma once

#include <cstdint>
#include <exception>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <function_ref.hpp>

#include "dvlnet/abstract_net.h"
#include "storm/storm_net.hpp"

namespace devilution::net {

class cdwrap : public abstract_net {
private:
	std::unique_ptr<abstract_net> dvlnet_wrap;
	std::map<event_type, SEVTHANDLER> registered_handlers;
	buffer_t game_init_info;
	std::optional<std::string> game_pw;
	tl::function_ref<std::unique_ptr<abstract_net>()> make_net_fn_;

	void reset();

public:
	explicit cdwrap(tl::function_ref<std::unique_ptr<abstract_net>()> makeNetFn)
	    : make_net_fn_(makeNetFn)
	{
		reset();
	}

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
	bool send_info_request() override;
	void clear_gamelist() override;
	std::vector<GameInfo> get_gamelist() override;
	void setup_password(std::string pw) override;
	void clear_password() override;

	virtual ~cdwrap() = default;
};

} // namespace devilution::net
