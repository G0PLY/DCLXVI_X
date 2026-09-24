/**
 * @file cursor.cpp
 *
 * Implementation of cursor tracking functionality.
 */
#include "cursor.h"

#include <cstdint>

#include <fmt/format.h>

#include "DiabloUI/diabloui.h"
#include "control.h"
#include "controls/plrctrls.h"
#include "doom.h"
#include "engine.h"
#include "engine/backbuffer_state.hpp"
#include "engine/load_cel.hpp"
#include "engine/load_clx.hpp"
#include "engine/point.hpp"
#include "engine/render/clx_render.hpp"
#include "engine/trn.hpp"
#include "hwcursor.hpp"
#include "inv.h"
#include "levels/trigs.h"
#include "missiles.h"
#include "options.h"
#include "qol/itemlabels.h"
#include "qol/stash.h"
#include "towners.h"
#include "track.h"
#include "utils/attributes.h"
#include "utils/language.h"
#include "utils/sdl_bilinear_scale.hpp"
#include "utils/surface_to_clx.hpp"
#include "utils/utf8.hpp"

namespace devilution {
namespace {
/** Cursor images CEL */
OptionalOwnedClxSpriteList pCursCels;
OptionalOwnedClxSpriteList pCursCels2;
/** Custom staff inventory sprites. Kept separate so the stock objcurs/objcurs2 layout is unchanged. */
OptionalOwnedClxSpriteList pStaffCursCels;
/** Custom armor inventory sprites. Kept separate so the stock objcurs/objcurs2 layout is unchanged. */
OptionalOwnedClxSpriteList pArmorCursCels;
/** Custom axe inventory sprites. Kept separate so the stock objcurs/objcurs2 layout is unchanged. */
OptionalOwnedClxSpriteList pAxeCursCels;
/** Custom light armor inventory sprites. Kept separate so the stock objcurs/objcurs2 layout is unchanged. */
OptionalOwnedClxSpriteList pLightArmorCursCels;
/** Custom sword inventory sprites. Kept separate so the stock objcurs/objcurs2 layout is unchanged. */
OptionalOwnedClxSpriteList pSwordCursCels;
/** Custom bow inventory sprites. */
OptionalOwnedClxSpriteList pBowCursCels;
/** Custom mace-family inventory sprites. */
OptionalOwnedClxSpriteList pMaceCursCels;
OptionalOwnedClxSpriteList pHelmCursCels;
OptionalOwnedClxSpriteList pShieldCursCels;
OptionalOwnedClxSpriteList pJewelryCursCels;

/** Maps from objcurs.cel frame number to frame width. */
const uint16_t InvItemWidth1[] = {
	// clang-format off
	// Cursors
	33, 32, 32, 32, 32, 32, 32, 32, 32, 32, 23,
	// Items
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
};
const uint16_t InvItemWidth2[] = {
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	2 * 28, 2 * 28, 1 * 28, 1 * 28, 1 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28
	// clang-format on
};
constexpr uint16_t InvItems1Size = sizeof(InvItemWidth1) / sizeof(InvItemWidth1[0]);
constexpr uint16_t InvItems2Size = sizeof(InvItemWidth2) / sizeof(InvItemWidth2[0]);

constexpr uint16_t FirstHellfireItemCursor = InvItems1Size - (CURSOR_FIRSTITEM - 1); // 168
constexpr uint16_t FirstCustomItemCursor = 229;
constexpr uint16_t CustomStaffCursorCount = 100;
constexpr uint16_t FirstCustomArmorItemCursor = FirstCustomItemCursor + CustomStaffCursorCount; // 329
constexpr uint16_t CustomArmorCursorCount = 100;
constexpr uint16_t FirstCustomAxeItemCursor = FirstCustomArmorItemCursor + CustomArmorCursorCount; // 429
constexpr uint16_t CustomAxeCursorCount = 100;
constexpr uint16_t FirstCustomLightArmorItemCursor = FirstCustomAxeItemCursor + CustomAxeCursorCount; // 529
constexpr uint16_t CustomLightArmorCursorCount = 100;
constexpr uint16_t FirstCustomSwordItemCursor = FirstCustomLightArmorItemCursor + CustomLightArmorCursorCount; // 629
constexpr uint16_t CustomSwordCursorCount = 100;
constexpr uint16_t FirstCustomBowItemCursor = FirstCustomSwordItemCursor + CustomSwordCursorCount; // 729
constexpr uint16_t CustomBowCursorCount = 100;
constexpr uint16_t FirstCustomMaceItemCursor = FirstCustomBowItemCursor + CustomBowCursorCount; // 829
constexpr uint16_t CustomMaceCursorCount = 100;
constexpr uint16_t FirstCustomCursorId = CURSOR_FIRSTITEM + FirstCustomItemCursor;                    // 241
constexpr uint16_t FirstCustomArmorCursorId = CURSOR_FIRSTITEM + FirstCustomArmorItemCursor;          // 341
constexpr uint16_t FirstCustomAxeCursorId = CURSOR_FIRSTITEM + FirstCustomAxeItemCursor;              // 441
constexpr uint16_t FirstCustomLightArmorCursorId = CURSOR_FIRSTITEM + FirstCustomLightArmorItemCursor; // 541
constexpr uint16_t FirstCustomSwordCursorId = CURSOR_FIRSTITEM + FirstCustomSwordItemCursor;          // 641
constexpr uint16_t FirstCustomBowCursorId = CURSOR_FIRSTITEM + FirstCustomBowItemCursor;              // 741
constexpr uint16_t FirstCustomMaceCursorId = CURSOR_FIRSTITEM + FirstCustomMaceItemCursor;            // 841
constexpr uint16_t FirstCustomHelmItemCursor = FirstCustomMaceItemCursor + CustomMaceCursorCount; // 929
constexpr uint16_t CustomHelmCursorCount = 120;
constexpr uint16_t FirstCustomHelmCursorId = CURSOR_FIRSTITEM + FirstCustomHelmItemCursor; // 941
constexpr uint16_t FirstCustomShieldItemCursor = FirstCustomHelmItemCursor + CustomHelmCursorCount; // 1049
constexpr uint16_t CustomShieldCursorCount = 100;
constexpr uint16_t FirstCustomShieldCursorId = CURSOR_FIRSTITEM + FirstCustomShieldItemCursor; // 1061
constexpr uint16_t FirstCustomJewelryItemCursor = FirstCustomShieldItemCursor + CustomShieldCursorCount; // 1149
constexpr uint16_t CustomJewelryCursorCount = 98;
constexpr uint16_t FirstCustomJewelryCursorId = CURSOR_FIRSTITEM + FirstCustomJewelryItemCursor; // 1161
constexpr uint16_t TotalLogicalItemCursors = FirstCustomJewelryItemCursor + CustomJewelryCursorCount; // 1247

const uint16_t CustomStaffWidth[CustomStaffCursorCount] = {
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28
};
const uint16_t CustomStaffHeight[CustomStaffCursorCount] = {
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28
};

/** Maps from objcurs.cel frame number to frame height. */
const uint16_t InvItemHeight1[InvItems1Size] = {
	// clang-format off
	// Cursors
	29, 32, 32, 32, 32, 32, 32, 32, 32, 32, 35,
	// Items
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28, 2 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
};
const uint16_t InvItemHeight2[InvItems2Size] = {
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28, 1 * 28,
	2 * 28, 2 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28, 3 * 28,
	3 * 28
	// clang-format on
};

OptionalOwnedClxSpriteList *HalfSizeItemSprites;
OptionalOwnedClxSpriteList *HalfSizeItemSpritesRed;

} // namespace

/** Current highlighted monster */
int pcursmonst = -1;

/** inv_item value */
int8_t pcursinvitem;
/** StashItem value */
uint16_t pcursstashitem;
/** Current highlighted item */
int8_t pcursitem;
/** Current highlighted object */
Object *ObjectUnderCursor;
/** Current highlighted player */
int8_t pcursplr;
/** Current highlighted tile position */
Point cursPosition;
/** Previously highlighted monster */
int pcurstemp;
/** Index of current cursor image */
int pcurs;

void InitCursor()
{
	assert(!pCursCels);
	pCursCels = LoadCel("data\\inv\\objcurs", InvItemWidth1);
	pStaffCursCels = LoadClx("data\\inv\\staffcurs.clx");
	pArmorCursCels = LoadClx("data\\inv\\armorcurs.clx");
	pAxeCursCels = LoadClx("data\\inv\\axecurs.clx");
	pLightArmorCursCels = LoadClx("data\\inv\\lightarmorcurs.clx");
	pSwordCursCels = LoadClx("data\\inv\\swordscurs.clx");
	pBowCursCels = LoadClx("data\\inv\\bowcurs.clx");
	pMaceCursCels = LoadClx("data\\inv\\macecurs.clx");
	pHelmCursCels = LoadClx("data\\inv\\helmcurs.clx");
	pShieldCursCels = LoadClx("data\\inv\\shieldcurs.clx");
	pJewelryCursCels = LoadClx("data\\inv\\jewelrycurs.clx");
	if (gbIsHellfire)
		pCursCels2 = LoadCel("data\\inv\\objcurs2", InvItemWidth2);
	ClearCursor();
}

void FreeCursor()
{
	pCursCels = std::nullopt;
	pCursCels2 = std::nullopt;
	pStaffCursCels = std::nullopt;
	pArmorCursCels = std::nullopt;
	pAxeCursCels = std::nullopt;
	pLightArmorCursCels = std::nullopt;
	pSwordCursCels = std::nullopt;
	pBowCursCels = std::nullopt;
	pMaceCursCels = std::nullopt;
	pHelmCursCels = std::nullopt;
	pShieldCursCels = std::nullopt;
	pJewelryCursCels = std::nullopt;
	ClearCursor();
}

ClxSprite GetInvItemSprite(int cursId)
{
	if (cursId >= FirstCustomJewelryCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomJewelryCursorId);
		assert(customIndex < CustomJewelryCursorCount);
		return (*pJewelryCursCels)[customIndex];
	}
	if (cursId >= FirstCustomShieldCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomShieldCursorId);
		assert(customIndex < CustomShieldCursorCount);
		return (*pShieldCursCels)[customIndex];
	}
	if (cursId >= FirstCustomHelmCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomHelmCursorId);
		assert(customIndex < CustomHelmCursorCount);
		return (*pHelmCursCels)[customIndex];
	}
	if (cursId >= FirstCustomMaceCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomMaceCursorId);
		assert(customIndex < CustomMaceCursorCount);
		return (*pMaceCursCels)[customIndex];
	}
	if (cursId >= FirstCustomBowCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomBowCursorId);
		assert(customIndex < CustomBowCursorCount);
		return (*pBowCursCels)[customIndex];
	}
	if (cursId >= FirstCustomSwordCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomSwordCursorId);
		assert(customIndex < CustomSwordCursorCount);
		return (*pSwordCursCels)[customIndex];
	}
	if (cursId >= FirstCustomLightArmorCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomLightArmorCursorId);
		assert(customIndex < CustomLightArmorCursorCount);
		return (*pLightArmorCursCels)[customIndex];
	}
	if (cursId >= FirstCustomAxeCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomAxeCursorId);
		assert(customIndex < CustomAxeCursorCount);
		return (*pAxeCursCels)[customIndex];
	}
	if (cursId >= FirstCustomArmorCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomArmorCursorId);
		assert(customIndex < CustomArmorCursorCount);
		return (*pArmorCursCels)[customIndex];
	}
	if (cursId >= FirstCustomCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomCursorId);
		assert(customIndex < CustomStaffCursorCount);
		return (*pStaffCursCels)[customIndex];
	}
	if (cursId <= InvItems1Size)
		return (*pCursCels)[cursId - 1];
	return (*pCursCels2)[cursId - InvItems1Size - 1];
}

size_t GetNumInvItems()
{
	return CURSOR_FIRSTITEM + TotalLogicalItemCursors - 1;
}

Size GetInvItemSize(int cursId)
{
	if (cursId >= FirstCustomJewelryCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomJewelryCursorId);
		assert(customIndex < CustomJewelryCursorCount);
		return { 1 * 28, 1 * 28 };
	}
	if (cursId >= FirstCustomShieldCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomShieldCursorId);
		assert(customIndex < CustomShieldCursorCount);
		return { 2 * 28, 3 * 28 };
	}
	if (cursId >= FirstCustomHelmCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomHelmCursorId);
		assert(customIndex < CustomHelmCursorCount);
		return { 2 * 28, 2 * 28 };
	}
	if (cursId >= FirstCustomMaceCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomMaceCursorId);
		assert(customIndex < CustomMaceCursorCount);

		// Hammer, War Hammer, and Flail are two-handed 2x3 items.
		if (customIndex >= 20 && customIndex < 80)
			return { 2 * 28, 3 * 28 };

		// Mace and Morning Star remain one-handed 1x3 items.
		return { 1 * 28, 3 * 28 };
	}
	if (cursId >= FirstCustomBowCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomBowCursorId);
		assert(customIndex < CustomBowCursorCount);
		return { 2 * 28, 3 * 28 };
	}
	if (cursId >= FirstCustomSwordCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomSwordCursorId);
		assert(customIndex < CustomSwordCursorCount);
		// Broad Sword, Bastard Sword, and Claymore use 1x3 inventory slots.
		// Great Sword and Two-Handed Sword remain 2x3.
		if (customIndex < 40 || customIndex >= 80)
			return { 1 * 28, 3 * 28 };
		return { 2 * 28, 3 * 28 };
	}
	if (cursId >= FirstCustomLightArmorCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomLightArmorCursorId);
		assert(customIndex < CustomLightArmorCursorCount);
		return { 2 * 28, 3 * 28 };
	}
	if (cursId >= FirstCustomAxeCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomAxeCursorId);
		assert(customIndex < CustomAxeCursorCount);
		return { 2 * 28, 3 * 28 };
	}
	if (cursId >= FirstCustomArmorCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomArmorCursorId);
		assert(customIndex < CustomArmorCursorCount);
		return { 2 * 28, 3 * 28 };
	}
	if (cursId >= FirstCustomCursorId) {
		const size_t customIndex = static_cast<size_t>(cursId - FirstCustomCursorId);
		assert(customIndex < CustomStaffCursorCount);
		return { CustomStaffWidth[customIndex], CustomStaffHeight[customIndex] };
	}
	const int i = cursId - 1;
	if (i >= InvItems1Size)
		return { InvItemWidth2[i - InvItems1Size], InvItemHeight2[i - InvItems1Size] };
	return { InvItemWidth1[i], InvItemHeight1[i] };
}

ClxSprite GetHalfSizeItemSprite(int cursId)
{
	return (*HalfSizeItemSprites[cursId])[0];
}

ClxSprite GetHalfSizeItemSpriteRed(int cursId)
{
	return (*HalfSizeItemSpritesRed[cursId])[0];
}

void CreateHalfSizeItemSprites()
{
	if (HalfSizeItemSprites != nullptr)
		return;
	const int numInvItems = TotalLogicalItemCursors;
	HalfSizeItemSprites = new OptionalOwnedClxSpriteList[numInvItems];
	HalfSizeItemSpritesRed = new OptionalOwnedClxSpriteList[numInvItems];
	const uint8_t *redTrn = GetInfravisionTRN();

	constexpr int MaxWidth = 28 * 3;
	constexpr int MaxHeight = 28 * 3;
	OwnedSurface ownedItemSurface { MaxWidth, MaxHeight };
	OwnedSurface ownedHalfSurface { MaxWidth / 2, MaxHeight / 2 };

	const auto createHalfSize = [&, redTrn](const ClxSprite itemSprite, size_t outputIndex) {
		if (itemSprite.width() <= 28 && itemSprite.height() <= 28) {
			// Skip creating half-size sprites for 1x1 items because we always render them at full size anyway.
			return;
		}
		const Surface itemSurface = ownedItemSurface.subregion(0, 0, itemSprite.width(), itemSprite.height());
		SDL_Rect itemSurfaceRect = MakeSdlRect(0, 0, itemSurface.w(), itemSurface.h());
		SDL_SetClipRect(itemSurface.surface, &itemSurfaceRect);
		SDL_FillRect(itemSurface.surface, nullptr, 1);
		ClxDraw(itemSurface, { 0, itemSurface.h() }, itemSprite);

		const Surface halfSurface = ownedHalfSurface.subregion(0, 0, itemSurface.w() / 2, itemSurface.h() / 2);
		SDL_Rect halfSurfaceRect = MakeSdlRect(0, 0, halfSurface.w(), halfSurface.h());
		SDL_SetClipRect(halfSurface.surface, &halfSurfaceRect);
		BilinearDownscaleByHalf8(itemSurface.surface, paletteTransparencyLookup, halfSurface.surface, 1);
		HalfSizeItemSprites[outputIndex].emplace(SurfaceToClx(halfSurface, 1, 1));

		SDL_FillRect(itemSurface.surface, nullptr, 1);
		ClxDrawTRN(itemSurface, { 0, itemSurface.h() }, itemSprite, redTrn);
		BilinearDownscaleByHalf8(itemSurface.surface, paletteTransparencyLookup, halfSurface.surface, 1);
		HalfSizeItemSpritesRed[outputIndex].emplace(SurfaceToClx(halfSurface, 1, 1));
	};

	// Half-size arrays are indexed by logical item cursor (_iCurs), not by physical CLX frame.
	for (size_t logicalCursor = 0; logicalCursor < FirstHellfireItemCursor; ++logicalCursor) {
		const size_t physicalFrame = (CURSOR_FIRSTITEM - 1) + logicalCursor;
		createHalfSize((*pCursCels)[physicalFrame], logicalCursor);
	}
	if (gbIsHellfire) {
		for (size_t i = 0; i < InvItems2Size; ++i)
			createHalfSize((*pCursCels2)[i], FirstHellfireItemCursor + i);
	}
	for (size_t i = 0; i < CustomStaffCursorCount; ++i)
		createHalfSize((*pStaffCursCels)[i], FirstCustomItemCursor + i);

	for (size_t i = 0; i < CustomArmorCursorCount; ++i)
		createHalfSize((*pArmorCursCels)[i], FirstCustomArmorItemCursor + i);

	for (size_t i = 0; i < CustomAxeCursorCount; ++i)
		createHalfSize((*pAxeCursCels)[i], FirstCustomAxeItemCursor + i);

	for (size_t i = 0; i < CustomLightArmorCursorCount; ++i)
		createHalfSize((*pLightArmorCursCels)[i], FirstCustomLightArmorItemCursor + i);

	for (size_t i = 0; i < CustomSwordCursorCount; ++i)
		createHalfSize((*pSwordCursCels)[i], FirstCustomSwordItemCursor + i);

	for (size_t i = 0; i < CustomBowCursorCount; ++i)
		createHalfSize((*pBowCursCels)[i], FirstCustomBowItemCursor + i);

	for (size_t i = 0; i < CustomMaceCursorCount; ++i)
		createHalfSize((*pMaceCursCels)[i], FirstCustomMaceItemCursor + i);
	for (size_t i = 0; i < CustomHelmCursorCount; ++i)
		createHalfSize((*pHelmCursCels)[i], FirstCustomHelmItemCursor + i);
	for (size_t i = 0; i < CustomShieldCursorCount; ++i)
		createHalfSize((*pShieldCursCels)[i], FirstCustomShieldItemCursor + i);
	for (size_t i = 0; i < CustomJewelryCursorCount; ++i)
		createHalfSize((*pJewelryCursCels)[i], FirstCustomJewelryItemCursor + i);
}

void FreeHalfSizeItemSprites()
{
	if (HalfSizeItemSprites != nullptr) {
		delete[] HalfSizeItemSprites;
		HalfSizeItemSprites = nullptr;
		delete[] HalfSizeItemSpritesRed;
		HalfSizeItemSpritesRed = nullptr;
	}
}

void DrawItem(const Item &item, const Surface &out, Point position, ClxSprite clx)
{
	const bool usable = !IsInspectingPlayer() ? item._iStatFlag : InspectPlayer->CanUseItem(item);
	if (usable) {
		ClxDraw(out, position, clx);
	} else {
		ClxDrawTRN(out, position, clx, GetInfravisionTRN());
	}
}

void ResetCursor()
{
	NewCursor(pcurs);
}

void NewCursor(const Item &item)
{
	if (item.isEmpty()) {
		NewCursor(CURSOR_HAND);
	} else {
		NewCursor(item._iCurs + CURSOR_FIRSTITEM);
	}
}

void NewCursor(int cursId)
{
	if (pcurs >= CURSOR_FIRSTITEM && cursId > CURSOR_HAND && cursId < CURSOR_HOURGLASS) {
		if (!TryDropItem()) {
			return;
		}
	}

	if (cursId < CURSOR_HOURGLASS && MyPlayer != nullptr) {
		MyPlayer->HoldItem.clear();
	}
	pcurs = cursId;

	if (IsHardwareCursorEnabled() && ControlDevice == ControlTypes::KeyboardAndMouse) {
		if (!ArtCursor && cursId == CURSOR_NONE)
			return;

		const CursorInfo newCursor = ArtCursor
		    ? CursorInfo::UserInterfaceCursor()
		    : CursorInfo::GameCursor(cursId);
		if (newCursor != GetCurrentCursorInfo())
			SetHardwareCursor(newCursor);
	}
}

void DrawSoftwareCursor(const Surface &out, Point position, int cursId)
{
	const ClxSprite sprite = GetInvItemSprite(cursId);
	if (!MyPlayer->HoldItem.isEmpty()) {
		const auto &heldItem = MyPlayer->HoldItem;
		ClxDrawOutline(out, GetOutlineColor(heldItem, true), position, sprite);
		DrawItem(heldItem, out, position, sprite);
	} else {
		ClxDraw(out, position, sprite);
	}
}

void InitLevelCursor()
{
	NewCursor(CURSOR_HAND);
	cursPosition = ViewPosition;
	pcurstemp = -1;
	pcursmonst = -1;
	ObjectUnderCursor = nullptr;
	pcursitem = -1;
	pcursstashitem = StashStruct::EmptyCell;
	pcursplr = -1;
	ClearCursor();
}

void CheckTown()
{
	for (auto &missile : Missiles) {
		if (missile._mitype == MissileID::TownPortal) {
			if (EntranceBoundaryContains(missile.position.tile, cursPosition)) {
				trigflag = true;
				InfoString = _("Town Portal");
				AddPanelString(fmt::format(fmt::runtime(_("from {:s}")), Players[missile._misource]._pName));
				cursPosition = missile.position.tile;
			}
		}
		if (missile._mitype == MissileID::RedPortal) {
			if (EntranceBoundaryContains(missile.position.tile, cursPosition)) {
				trigflag = true;
				InfoString = _("Town Portal");
				AddPanelString(fmt::format(fmt::runtime(_("from {:s}")), Players[missile._misource]._pName));
				cursPosition = missile.position.tile;
			}
		}
	}
}

void CheckRportal()
{
	for (auto &missile : Missiles) {
		if (missile._mitype == MissileID::RedPortal) {
			if (EntranceBoundaryContains(missile.position.tile, cursPosition)) {
				trigflag = true;
				InfoString = _("Portal to");
				AddPanelString(!setlevel ? _("The Unholy Altar") : _("level 15"));
				cursPosition = missile.position.tile;
			}
		}
	}
}

void CheckCursMove()
{
	if (IsItemLabelHighlighted())
		return;

	int sx = MousePosition.x;
	int sy = MousePosition.y;

	if (CanPanelsCoverView()) {
		if (IsLeftPanelOpen()) {
			sx -= GetScreenWidth() / 4;
		} else if (IsRightPanelOpen()) {
			sx += GetScreenWidth() / 4;
		}
	}
	const Rectangle &mainPanel = GetMainPanel();
	if (mainPanel.contains(MousePosition) && track_isscrolling()) {
		sy = mainPanel.position.y - 1;
	}

	if (*sgOptions.Graphics.zoom) {
		sx /= 2;
		sy /= 2;
	}

	// Adjust by player offset and tile grid alignment
	int xo = 0;
	int yo = 0;
	CalcTileOffset(&xo, &yo);
	sx += xo;
	sy += yo;

	const Player &myPlayer = *MyPlayer;

	if (myPlayer.isWalking()) {
		Displacement offset = GetOffsetForWalking(myPlayer.AnimInfo, myPlayer._pdir, true);
		sx -= offset.deltaX;
		sy -= offset.deltaY;

		// Predict the next frame when walking to avoid input jitter
		DisplacementOf<int16_t> offset2 = myPlayer.position.CalculateWalkingOffsetShifted8(myPlayer._pdir, myPlayer.AnimInfo);
		DisplacementOf<int16_t> velocity = myPlayer.position.GetWalkingVelocityShifted8(myPlayer._pdir, myPlayer.AnimInfo);
		int fx = offset2.deltaX / 256;
		int fy = offset2.deltaY / 256;
		fx -= (offset2.deltaX + velocity.deltaX) / 256;
		fy -= (offset2.deltaY + velocity.deltaY) / 256;

		sx -= fx;
		sy -= fy;
	}

	// Convert to tile grid
	int mx = ViewPosition.x;
	int my = ViewPosition.y;

	int columns = 0;
	int rows = 0;
	TilesInView(&columns, &rows);
	int lrow = rows - RowsCoveredByPanel();

	// Center player tile on screen
	ShiftGrid(&mx, &my, -columns / 2, -lrow / 2);

	// Align grid
	if ((columns % 2) == 0 && (lrow % 2) == 0) {
		sy += TILE_HEIGHT / 2;
	} else if ((columns % 2) != 0 && (lrow % 2) != 0) {
		sx -= TILE_WIDTH / 2;
	} else if ((columns % 2) != 0 && (lrow % 2) == 0) {
		my++;
	}

	if (*sgOptions.Graphics.zoom) {
		sy -= TILE_HEIGHT / 4;
	}

	int tx = sx / TILE_WIDTH;
	int ty = sy / TILE_HEIGHT;
	ShiftGrid(&mx, &my, tx, ty);

	// Shift position to match diamond grid aligment
	int px = sx % TILE_WIDTH;
	int py = sy % TILE_HEIGHT;

	// Shift position to match diamond grid aligment
	bool flipy = py < (px / 2);
	if (flipy) {
		my--;
	}
	bool flipx = py >= TILE_HEIGHT - (px / 2);
	if (flipx) {
		mx++;
	}

	mx = clamp(mx, 0, MAXDUNX - 1);
	my = clamp(my, 0, MAXDUNY - 1);

	const Point currentTile { mx, my };

	// While holding the button down we should retain target (but potentially lose it if it dies, goes out of view, etc)
	if ((sgbMouseDown != CLICK_NONE || ControllerActionHeld != GameActionType_NONE) && IsNoneOf(LastMouseButtonAction, MouseActionType::None, MouseActionType::Attack, MouseActionType::Spell)) {
		InvalidateTargets();

		if (pcursmonst == -1 && ObjectUnderCursor == nullptr && pcursitem == -1 && pcursinvitem == -1 && pcursstashitem == StashStruct::EmptyCell && pcursplr == -1) {
			cursPosition = { mx, my };
			CheckTrigForce();
			CheckTown();
			CheckRportal();
		}
		return;
	}

	bool flipflag = (flipy && flipx) || ((flipy || flipx) && px < TILE_WIDTH / 2);

	pcurstemp = pcursmonst;
	pcursmonst = -1;
	ObjectUnderCursor = nullptr;
	pcursitem = -1;
	if (pcursinvitem != -1) {
		RedrawComponent(PanelDrawComponent::Belt);
	}
	pcursinvitem = -1;
	pcursstashitem = StashStruct::EmptyCell;
	pcursplr = -1;
	ShowUniqueItemInfoBox = false;
	panelflag = false;
	trigflag = false;

	if (myPlayer._pInvincible) {
		return;
	}
	if (!myPlayer.HoldItem.isEmpty() || spselflag) {
		cursPosition = { mx, my };
		return;
	}
	if (mainPanel.contains(MousePosition)) {
		CheckPanelInfo();
		return;
	}
	if (DoomFlag) {
		return;
	}
	if (invflag && GetRightPanel().contains(MousePosition)) {
		pcursinvitem = CheckInvHLight();
		return;
	}
	if (IsStashOpen && GetLeftPanel().contains(MousePosition)) {
		pcursstashitem = CheckStashHLight(MousePosition);
	}
	if (sbookflag && GetRightPanel().contains(MousePosition)) {
		return;
	}
	if (IsLeftPanelOpen() && GetLeftPanel().contains(MousePosition)) {
		return;
	}

	if (leveltype != DTYPE_TOWN) {
		if (pcurstemp != -1) {
			if (!flipflag && mx + 2 < MAXDUNX && my + 1 < MAXDUNY && dMonster[mx + 2][my + 1] != 0 && IsTileLit({ mx + 2, my + 1 })) {
				const uint16_t monsterId = abs(dMonster[mx + 2][my + 1]) - 1;
				if (monsterId == pcurstemp && Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 4) != 0) {
					cursPosition = Point { mx, my } + Displacement { 2, 1 };
					pcursmonst = monsterId;
				}
			}
			if (flipflag && mx + 1 < MAXDUNX && my + 2 < MAXDUNY && dMonster[mx + 1][my + 2] != 0 && IsTileLit({ mx + 1, my + 2 })) {
				const uint16_t monsterId = abs(dMonster[mx + 1][my + 2]) - 1;
				if (monsterId == pcurstemp && Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 4) != 0) {
					cursPosition = Point { mx, my } + Displacement { 1, 2 };
					pcursmonst = monsterId;
				}
			}
			if (mx + 2 < MAXDUNX && my + 2 < MAXDUNY && dMonster[mx + 2][my + 2] != 0 && IsTileLit({ mx + 2, my + 2 })) {
				const uint16_t monsterId = abs(dMonster[mx + 2][my + 2]) - 1;
				if (monsterId == pcurstemp && Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 4) != 0) {
					cursPosition = Point { mx, my } + Displacement { 2, 2 };
					pcursmonst = monsterId;
				}
			}
			if (mx + 1 < MAXDUNX && !flipflag && dMonster[mx + 1][my] != 0 && IsTileLit({ mx + 1, my })) {
				const uint16_t monsterId = abs(dMonster[mx + 1][my]) - 1;
				if (monsterId == pcurstemp && Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 2) != 0) {
					cursPosition = Point { mx, my } + Displacement { 1, 0 };
					pcursmonst = monsterId;
				}
			}
			if (my + 1 < MAXDUNY && flipflag && dMonster[mx][my + 1] != 0 && IsTileLit({ mx, my + 1 })) {
				const uint16_t monsterId = abs(dMonster[mx][my + 1]) - 1;
				if (monsterId == pcurstemp && Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 2) != 0) {
					cursPosition = Point { mx, my } + Displacement { 0, 1 };
					pcursmonst = monsterId;
				}
			}
			if (dMonster[mx][my] != 0 && IsTileLit({ mx, my })) {
				const uint16_t monsterId = abs(dMonster[mx][my]) - 1;
				if (monsterId == pcurstemp && Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 1) != 0) {
					cursPosition = { mx, my };
					pcursmonst = monsterId;
				}
			}
			if (mx + 1 < MAXDUNX && my + 1 < MAXDUNY && dMonster[mx + 1][my + 1] != 0 && IsTileLit({ mx + 1, my + 1 })) {
				const uint16_t monsterId = abs(dMonster[mx + 1][my + 1]) - 1;
				if (monsterId == pcurstemp && Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 2) != 0) {
					cursPosition = Point { mx, my } + Displacement { 1, 1 };
					pcursmonst = monsterId;
				}
			}
			if (pcursmonst != -1 && (Monsters[pcursmonst].flags & MFLAG_HIDDEN) != 0) {
				pcursmonst = -1;
				cursPosition = { mx, my };
			}
			if (pcursmonst != -1 && Monsters[pcursmonst].isPlayerMinion()) {
				pcursmonst = -1;
			}
			if (pcursmonst != -1) {
				return;
			}
		}
		if (!flipflag && mx + 2 < MAXDUNX && my + 1 < MAXDUNY && dMonster[mx + 2][my + 1] != 0 && IsTileLit({ mx + 2, my + 1 })) {
			int monsterId = abs(dMonster[mx + 2][my + 1]) - 1;
			if (Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 4) != 0) {
				cursPosition = Point { mx, my } + Displacement { 2, 1 };
				pcursmonst = monsterId;
			}
		}
		if (flipflag && mx + 1 < MAXDUNX && my + 2 < MAXDUNY && dMonster[mx + 1][my + 2] != 0 && IsTileLit({ mx + 1, my + 2 })) {
			const uint16_t monsterId = abs(dMonster[mx + 1][my + 2]) - 1;
			if (Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 4) != 0) {
				cursPosition = Point { mx, my } + Displacement { 1, 2 };
				pcursmonst = monsterId;
			}
		}
		if (mx + 2 < MAXDUNX && my + 2 < MAXDUNY && dMonster[mx + 2][my + 2] != 0 && IsTileLit({ mx + 2, my + 2 })) {
			const uint16_t monsterId = abs(dMonster[mx + 2][my + 2]) - 1;
			if (Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 4) != 0) {
				cursPosition = Point { mx, my } + Displacement { 2, 2 };
				pcursmonst = monsterId;
			}
		}
		if (!flipflag && mx + 1 < MAXDUNX && dMonster[mx + 1][my] != 0 && IsTileLit({ mx + 1, my })) {
			const uint16_t monsterId = abs(dMonster[mx + 1][my]) - 1;
			if (Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 2) != 0) {
				cursPosition = Point { mx, my } + Displacement { 1, 0 };
				pcursmonst = monsterId;
			}
		}
		if (flipflag && my + 1 < MAXDUNY && dMonster[mx][my + 1] != 0 && IsTileLit({ mx, my + 1 })) {
			const uint16_t monsterId = abs(dMonster[mx][my + 1]) - 1;
			if (Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 2) != 0) {
				cursPosition = Point { mx, my } + Displacement { 0, 1 };
				pcursmonst = monsterId;
			}
		}
		if (dMonster[mx][my] != 0 && IsTileLit({ mx, my })) {
			const uint16_t monsterId = abs(dMonster[mx][my]) - 1;
			if (Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 1) != 0) {
				cursPosition = { mx, my };
				pcursmonst = monsterId;
			}
		}
		if (mx + 1 < MAXDUNX && my + 1 < MAXDUNY && dMonster[mx + 1][my + 1] != 0 && IsTileLit({ mx + 1, my + 1 })) {
			const uint16_t monsterId = abs(dMonster[mx + 1][my + 1]) - 1;
			if (Monsters[monsterId].hitPoints >> 6 > 0 && (Monsters[monsterId].data().selectionType & 2) != 0) {
				cursPosition = Point { mx, my } + Displacement { 1, 1 };
				pcursmonst = monsterId;
			}
		}
		if (pcursmonst != -1 && (Monsters[pcursmonst].flags & MFLAG_HIDDEN) != 0) {
			pcursmonst = -1;
			cursPosition = { mx, my };
		}
		if (pcursmonst != -1 && (Monsters[pcursmonst].isPlayerMinion() || IsAnyOf(pcurs, CURSOR_HEALOTHER, CURSOR_RESURRECT))) {
			pcursmonst = -1;
		}
	} else {
		if (!flipflag && mx + 1 < MAXDUNX && dMonster[mx + 1][my] > 0) {
			pcursmonst = dMonster[mx + 1][my] - 1;
			cursPosition = Point { mx, my } + Displacement { 1, 0 };
		}
		if (flipflag && my + 1 < MAXDUNY && dMonster[mx][my + 1] > 0) {
			pcursmonst = dMonster[mx][my + 1] - 1;
			cursPosition = Point { mx, my } + Displacement { 0, 1 };
		}
		if (dMonster[mx][my] > 0) {
			pcursmonst = dMonster[mx][my] - 1;
			cursPosition = { mx, my };
		}
		if (mx + 1 < MAXDUNX && my + 1 < MAXDUNY && dMonster[mx + 1][my + 1] > 0) {
			pcursmonst = dMonster[mx + 1][my + 1] - 1;
			cursPosition = Point { mx, my } + Displacement { 1, 1 };
		}
	}

	if (pcursmonst == -1) {
		if (!flipflag && mx + 1 < MAXDUNX && dPlayer[mx + 1][my] != 0) {
			const uint8_t playerId = abs(dPlayer[mx + 1][my]) - 1;
			Player &player = Players[playerId];
			if (&player != MyPlayer && player._pHitPoints != 0) {
				cursPosition = Point { mx, my } + Displacement { 1, 0 };
				pcursplr = static_cast<int8_t>(playerId);
			}
		}
		if (flipflag && my + 1 < MAXDUNY && dPlayer[mx][my + 1] != 0) {
			const uint8_t playerId = abs(dPlayer[mx][my + 1]) - 1;
			Player &player = Players[playerId];
			if (&player != MyPlayer && player._pHitPoints != 0) {
				cursPosition = Point { mx, my } + Displacement { 0, 1 };
				pcursplr = static_cast<int8_t>(playerId);
			}
		}
		if (dPlayer[mx][my] != 0) {
			const uint8_t playerId = abs(dPlayer[mx][my]) - 1;
			if (playerId != MyPlayerId) {
				cursPosition = { mx, my };
				pcursplr = static_cast<int8_t>(playerId);
			}
		}
		if (TileContainsDeadPlayer({ mx, my })) {
			for (const Player &player : Players) {
				if (player.position.tile == Point { mx, my } && &player != MyPlayer) {
					cursPosition = { mx, my };
					pcursplr = static_cast<int8_t>(player.getId());
				}
			}
		}
		if (pcurs == CURSOR_RESURRECT) {
			for (int xx = -1; xx < 2; xx++) {
				for (int yy = -1; yy < 2; yy++) {
					if (TileContainsDeadPlayer({ mx + xx, my + yy })) {
						for (const Player &player : Players) {
							if (player.position.tile.x == mx + xx && player.position.tile.y == my + yy && &player != MyPlayer) {
								cursPosition = Point { mx, my } + Displacement { xx, yy };
								pcursplr = static_cast<int8_t>(player.getId());
							}
						}
					}
				}
			}
		}
		if (mx + 1 < MAXDUNX && my + 1 < MAXDUNY && dPlayer[mx + 1][my + 1] != 0) {
			const uint8_t playerId = abs(dPlayer[mx + 1][my + 1]) - 1;
			const Player &player = Players[playerId];
			if (&player != MyPlayer && player._pHitPoints != 0) {
				cursPosition = Point { mx, my } + Displacement { 1, 1 };
				pcursplr = static_cast<int8_t>(playerId);
			}
		}
	}
	if (pcursmonst == -1 && pcursplr == -1) {
		// No monsters or players under the cursor, try find an object starting with the tile below the current tile (tall
		//  objects like doors)
		Point testPosition = currentTile + Direction::South;
		Object *object = FindObjectAtPosition(testPosition);

		if (object == nullptr || object->_oSelFlag < 2) {
			// Either no object or can't interact from the test position, try the current tile
			testPosition = currentTile;
			object = FindObjectAtPosition(testPosition);

			if (object == nullptr || IsNoneOf(object->_oSelFlag, 1, 3)) {
				// Still no object (that could be activated from this position), try the tile to the bottom left or right
				//  (whichever is closest to the cursor as determined when we set flipflag earlier)
				testPosition = currentTile + (flipflag ? Direction::SouthWest : Direction::SouthEast);
				object = FindObjectAtPosition(testPosition);

				if (object != nullptr && object->_oSelFlag < 2) {
					// Found an object but it's not in range, clear the pointer
					object = nullptr;
				}
			}
		}
		if (object != nullptr) {
			// found object that can be activated with the given cursor position
			cursPosition = testPosition;
			ObjectUnderCursor = object;
		}
	}
	if (pcursplr == -1 && ObjectUnderCursor == nullptr && pcursmonst == -1) {
		if (!flipflag && mx + 1 < MAXDUNX && dItem[mx + 1][my] > 0) {
			const uint8_t itemId = dItem[mx + 1][my] - 1;
			if (Items[itemId]._iSelFlag >= 2) {
				cursPosition = Point { mx, my } + Displacement { 1, 0 };
				pcursitem = static_cast<int8_t>(itemId);
			}
		}
		if (flipflag && my + 1 < MAXDUNY && dItem[mx][my + 1] > 0) {
			const uint8_t itemId = dItem[mx][my + 1] - 1;
			if (Items[itemId]._iSelFlag >= 2) {
				cursPosition = Point { mx, my } + Displacement { 0, 1 };
				pcursitem = static_cast<int8_t>(itemId);
			}
		}
		if (dItem[mx][my] > 0) {
			const uint8_t itemId = dItem[mx][my] - 1;
			if (Items[itemId]._iSelFlag == 1 || Items[itemId]._iSelFlag == 3) {
				cursPosition = { mx, my };
				pcursitem = static_cast<int8_t>(itemId);
			}
		}
		if (mx + 1 < MAXDUNX && my + 1 < MAXDUNY && dItem[mx + 1][my + 1] > 0) {
			const uint8_t itemId = dItem[mx + 1][my + 1] - 1;
			if (Items[itemId]._iSelFlag >= 2) {
				cursPosition = Point { mx, my } + Displacement { 1, 1 };
				pcursitem = static_cast<int8_t>(itemId);
			}
		}
		if (pcursitem == -1) {
			cursPosition = { mx, my };
			CheckTrigForce();
			CheckTown();
			CheckRportal();
		}
	}

	if (pcurs == CURSOR_IDENTIFY) {
		ObjectUnderCursor = nullptr;
		pcursmonst = -1;
		pcursitem = -1;
		cursPosition = { mx, my };
	}
	if (pcursmonst != -1 && leveltype != DTYPE_TOWN && Monsters[pcursmonst].isPlayerMinion()) {
		pcursmonst = -1;
	}
}

} // namespace devilution
