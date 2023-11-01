/**
 * @file towners.h
 *
 * Interface of functionality for loading and spawning towners.
 */
#pragma once

#include "utils/stdcompat/string_view.hpp"
#include <cstdint>
#include <memory>

#include "items.h"
#include "player.h"
#include "quests.h"
#include "utils/stdcompat/cstddef.hpp"

namespace devilution {

#define NUM_TOWNERS 16

enum _talker_id : uint8_t {
	TOWN_SMITH,
	TOWN_HEALER,
	TOWN_DEADGUY,
	TOWN_TAVERN,
	TOWN_STORY,
	TOWN_DRUNK,
	TOWN_WITCH,
	TOWN_BMAID,
	TOWN_PEGBOY,
	TOWN_COW,
	TOWN_FARMER,
	TOWN_GIRL,
	TOWN_COWFARM,
	NUM_TOWNER_TYPES,
};

struct Towner {
	OptionalOwnedClxSpriteList ownedAnim;
	OptionalClxSpriteList anim;
	/** Specifies the animation frame sequence. */
	const uint8_t *animOrder; // unowned
	void (*talk)(Player &player, Towner &towner);

	string_view name;

	/** Tile position of NPC */
	Point position;
	/** Randomly chosen topic for discussion (picked when loading into town) */
	_speech_id gossip;
	uint16_t _tAnimWidth;
	/** Tick length of each frame in the current animation */
	int16_t _tAnimDelay;
	/** Increases by one each game tick, counting how close we are to _pAnimDelay */
	int16_t _tAnimCnt;
	/** Number of frames in current animation */
	uint8_t _tAnimLen;
	/** Current frame of animation. */
	uint8_t _tAnimFrame;
	uint8_t _tAnimFrameCnt;
	uint8_t animOrderSize;
	_talker_id _ttype;

	ClxSprite currentSprite() const
	{
		return (*anim)[_tAnimFrame];
	}
};

extern Towner Towners[NUM_TOWNERS];

void InitTowners();
void FreeTownerGFX();
void ProcessTowners();
void TalkToTowner(Player &player, int t);

#ifdef _DEBUG
bool DebugTalkToTowner(std::string targetName);
#endif
extern _speech_id QuestDialogTable[NUM_TOWNER_TYPES][MAXQUESTS];

} // namespace devilution
