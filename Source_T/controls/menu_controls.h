#pragma once

#include <SDL.h>
#include <cstdint>
#include <vector>

namespace devilution {

enum MenuAction : uint8_t {
	MenuAction_NONE,
	MenuAction_SELECT,
	MenuAction_BACK,
	MenuAction_DELETE,

	MenuAction_UP,
	MenuAction_DOWN,
	MenuAction_LEFT,
	MenuAction_RIGHT,

	MenuAction_PAGE_UP,
	MenuAction_PAGE_DOWN,
};

std::vector<MenuAction> GetMenuActions(const SDL_Event &event);

/** Menu action from holding the left stick or DPad. */
MenuAction GetMenuHeldUpDownAction();

} // namespace devilution
