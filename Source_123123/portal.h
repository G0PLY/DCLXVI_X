/**
 * @file portal.h
 *
 * Interface of functionality for handling town portals.
 */
#pragma once

#include "engine/point.hpp"
#include "levels/gendung.h"

namespace devilution {

#define MAXPORTAL 4

struct Portal {
	bool open;
	Point position;
	int level;
	dungeon_type ltype;
	bool setlvl;
};

extern Portal Portals[MAXPORTAL];

void InitPortals();
void SetPortalStats(int i, bool o, Point position, int lvl, dungeon_type lvltype, bool isSetLevel);
void AddWarpMissile(int i, Point position, bool sync);
void SyncPortals();
void AddInTownPortal(int i);
void ActivatePortal(int i, Point position, int lvl, dungeon_type lvltype, bool sp);
void DeactivatePortal(int i);
bool PortalOnLevel(size_t i);
void RemovePortalMissile(int id);
void SetCurrentPortal(size_t p);
void GetPortalLevel();
void GetPortalLvlPos();
bool PosOkPortal(int lvl, Point position);

} // namespace devilution
