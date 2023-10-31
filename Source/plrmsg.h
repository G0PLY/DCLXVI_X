/**
 * @file plrmsg.h
 *
 * Interface of functionality for printing the ingame chat messages.
 */
#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include <SDL.h>

#include "DiabloUI/ui_flags.hpp"
#include "engine.h"
#include "player.h"

namespace devilution {

void plrmsg_delay(bool delay);
void EventPlrMsg(std::string_view text, UiFlags style = UiFlags::ColorWhitegold);
void SendPlrMsg(Player &player, std::string_view text);
void InitPlrMsg();
void DrawPlrMsg(const Surface &out);

} // namespace devilution
