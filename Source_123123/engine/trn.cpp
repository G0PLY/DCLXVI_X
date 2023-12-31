#include "engine/trn.hpp"

#include <cstdint>
#include <unordered_map>

#include <fmt/format.h>

#ifdef _DEBUG
#include "debug.h"
#endif
#include "engine/load_file.hpp"
#include "lighting.h"

namespace devilution {

uint8_t *GetInfravisionTRN()
{
	return InfravisionTable.data();
}

uint8_t *GetStoneTRN()
{
	return StoneTable.data();
}

uint8_t *GetPauseTRN()
{
	return PauseTable.data();
}

std::optional<std::array<uint8_t, 256>> GetClassTRN(Player &player)
{
	std::array<uint8_t, 256> trn;
	const char *path;

	switch (player._pClass) {
	case HeroClass::Warrior:
		path = "plrgfx\\warrior.trn";
		break;
	case HeroClass::Rogue:
		path = "plrgfx\\rogue.trn";
		break;
	case HeroClass::Sorcerer:
		path = "plrgfx\\sorcerer.trn";
		break;
	case HeroClass::Monk:
		path = "plrgfx\\monk.trn";
		break;
	case HeroClass::Bard:
		path = "plrgfx\\bard.trn";
		break;
	case HeroClass::Barbarian:
		path = "plrgfx\\barbarian.trn";
		break;
	case HeroClass::Paladin:
		//path = "plrgfx\\monk.trn";
		path = "plrgfx\\warrior.trn";
		break;
	case HeroClass::Assassin:
		path = "plrgfx\\rogue.trn";
		break;
	case HeroClass::Battlemage:
		path = "plrgfx\\sorcerer.trn";
		break;
	case HeroClass::Bloodmage:
		path = "plrgfx\\sorcerer.trn";
		break;
	case HeroClass::Templar:
		path = "plrgfx\\warrior.trn";
		break;
	case HeroClass::Witch:
		path = "plrgfx\\rogue.trn";
		break;
	case HeroClass::Warlock:
		path = "plrgfx\\sorcerer.trn";
		break;
	case HeroClass::Kabbalist:
		path = "plrgfx\\monk.trn";
		break;
	case HeroClass::Traveler:
		path = "plrgfx\\monk.trn";
		break;
	case HeroClass::Sage:
		path = "plrgfx\\monk.trn";
		break;
	case HeroClass::Cleric:
		path = "plrgfx\\warrior.trn";
		break;
	}

#ifdef _DEBUG
	if (!debugTRN.empty()) {
		path = debugTRN.c_str();
	}
#endif
	if (LoadOptionalFileInMem(path, &trn[0], 256)) {
		return trn;
	}
	return std::nullopt;
}

} // namespace devilution
