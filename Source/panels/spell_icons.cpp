#include "panels/spell_icons.hpp"

#include <cstdint>

#include "engine.h"
#include "engine/load_cel.hpp"
#include "engine/load_clx.hpp"
#include "engine/palette.h"
#include "engine/render/clx_render.hpp"
#include "init.h"
#include "utils/stdcompat/optional.hpp"

namespace devilution {

namespace {

#ifdef UNPACKED_MPQS
OptionalOwnedClxSpriteList LargeSpellIconsBackground;
OptionalOwnedClxSpriteList SmallSpellIconsBackground;
#endif

OptionalOwnedClxSpriteList SmallSpellIcons;
OptionalOwnedClxSpriteList LargeSpellIcons;

uint8_t SplTransTbl[256];

/** Maps from SpellID to spelicon.cel frame number. */
const uint8_t SpellITbl[] = {
	26, // NULL
	0, // firebolt
	1, // healing
	2, // lightning
	3, //flash
	4, //identify
	5, //firewall
	6, //townp
	7, //stonec
	8, // infravision
	27, // phase ()
	12, // manashi
	11, // fireball
	17, // guardian
	15, // chainl
	13, // flamewave
	21, // dooomserp (staticfield)
	33, // bloodritual (bola aka knockback)
	10, // nova
	19, // invis (aimedshot)
	14, // inferno
	20, //golem
	22, //rage
	23, // tele
	24, // apoc
	21, // etherealize
	25, //item rep
	28, //staff rech
	36, //trap dis
	37, //elemental
	38, //chargedbolt
	41, //holybolt
	40, //resurrect
	39, //telekinesis
	9, // healother
	35, //bloodstar
	29, //bonespirit
	32, // manaregen (reflect)
	24, // healthregen
	31, // dmgreduct ^
	35, // witch bloodstar   /// new spells insert below this
	42, // smite (was mana #50) (divine intervention)
	50, // the magi
	49, // jester
	16, // lightningwall (?changed was 45)
	46, // immolation
	41, // warp (smite^)
	44, // reflect
	47, // berserk
	45, // rune F (lightningwall)
	46, // rune L (immolation)
	34, // rune N
	34, // rune I
	34, // rune S
	34, // Manaregen
	24, // Healthregen
};

} // namespace

void LoadLargeSpellIcons()
{
	if (!gbIsHellfire) {
#ifdef UNPACKED_MPQS
		LargeSpellIcons = LoadClx("ctrlpan\\spelicon_fg.clx");
		LargeSpellIconsBackground = LoadClx("ctrlpan\\spelicon_bg.clx");
#else
		LargeSpellIcons = LoadCel("ctrlpan\\spelicon", SPLICONLENGTH);
#endif
	} else {
#ifdef UNPACKED_MPQS
		LargeSpellIcons = LoadClx("data\\spelicon_fg.clx");
		LargeSpellIconsBackground = LoadClx("data\\spelicon_bg.clx");
#else
		LargeSpellIcons = LoadCel("data\\spelicon", SPLICONLENGTH);
#endif
	}
	SetSpellTrans(SpellType::Skill);
}

void FreeLargeSpellIcons()
{
#ifdef UNPACKED_MPQS
	LargeSpellIconsBackground = std::nullopt;
#endif
	LargeSpellIcons = std::nullopt;
}

void LoadSmallSpellIcons()
{
#ifdef UNPACKED_MPQS
	SmallSpellIcons = LoadClx("data\\spelli2_fg.clx");
	SmallSpellIconsBackground = LoadClx("data\\spelli2_bg.clx");
#else
	SmallSpellIcons = LoadCel("data\\spelli2", 37);
#endif
}

void FreeSmallSpellIcons()
{
#ifdef UNPACKED_MPQS
	SmallSpellIconsBackground = std::nullopt;
#endif
	SmallSpellIcons = std::nullopt;
}

void DrawLargeSpellIcon(const Surface &out, Point position, SpellID spell)
{
#ifdef UNPACKED_MPQS
	ClxDrawTRN(out, position, (*LargeSpellIconsBackground)[0], SplTransTbl);
#endif
	ClxDrawTRN(out, position, (*LargeSpellIcons)[SpellITbl[static_cast<int8_t>(spell)]], SplTransTbl);
}

void DrawSmallSpellIcon(const Surface &out, Point position, SpellID spell)
{
#ifdef UNPACKED_MPQS
	ClxDrawTRN(out, position, (*SmallSpellIconsBackground)[0], SplTransTbl);
#endif
	ClxDrawTRN(out, position, (*SmallSpellIcons)[SpellITbl[static_cast<int8_t>(spell)]], SplTransTbl);
}

void DrawLargeSpellIconBorder(const Surface &out, Point position, uint8_t color)
{
	const int width = (*LargeSpellIcons)[0].width();
	const int height = (*LargeSpellIcons)[0].height();
	UnsafeDrawBorder2px(out, Rectangle { Point { position.x, position.y - height + 1 }, Size { width, height } }, color);
}

void DrawSmallSpellIconBorder(const Surface &out, Point position)
{
	const int width = (*SmallSpellIcons)[0].width();
	const int height = (*SmallSpellIcons)[0].height();
	UnsafeDrawBorder2px(out, Rectangle { Point { position.x, position.y - height + 1 }, Size { width, height } }, SplTransTbl[PAL8_YELLOW + 2]);
}

void SetSpellTrans(SpellType t)
{
	if (t == SpellType::Skill) {
		for (int i = 0; i < 128; i++)
			SplTransTbl[i] = i;
	}
	for (int i = 128; i < 256; i++)
		SplTransTbl[i] = i;
	SplTransTbl[255] = 0;

	switch (t) {
	case SpellType::Spell:
		SplTransTbl[PAL8_YELLOW] = PAL16_BLUE + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_BLUE + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_BLUE + 5;
		for (int i = PAL16_BLUE; i < PAL16_BLUE + 16; i++) {
			SplTransTbl[PAL16_BEIGE - PAL16_BLUE + i] = i;
			SplTransTbl[PAL16_YELLOW - PAL16_BLUE + i] = i;
			SplTransTbl[PAL16_ORANGE - PAL16_BLUE + i] = i;
		}
		break;
	case SpellType::Scroll:
		SplTransTbl[PAL8_YELLOW] = PAL16_BEIGE + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_BEIGE + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_BEIGE + 5;
		for (int i = PAL16_BEIGE; i < PAL16_BEIGE + 16; i++) {
			SplTransTbl[PAL16_YELLOW - PAL16_BEIGE + i] = i;
			SplTransTbl[PAL16_ORANGE - PAL16_BEIGE + i] = i;
		}
		break;
	case SpellType::Charges:
		SplTransTbl[PAL8_YELLOW] = PAL16_ORANGE + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_ORANGE + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_ORANGE + 5;
		for (int i = PAL16_ORANGE; i < PAL16_ORANGE + 16; i++) {
			SplTransTbl[PAL16_BEIGE - PAL16_ORANGE + i] = i;
			SplTransTbl[PAL16_YELLOW - PAL16_ORANGE + i] = i;
		}
		break;
	case SpellType::Invalid:
		SplTransTbl[PAL8_YELLOW] = PAL16_GRAY + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_GRAY + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_GRAY + 5;
		for (int i = PAL16_GRAY; i < PAL16_GRAY + 15; i++) {
			SplTransTbl[PAL16_BEIGE - PAL16_GRAY + i] = i;
			SplTransTbl[PAL16_YELLOW - PAL16_GRAY + i] = i;
			SplTransTbl[PAL16_ORANGE - PAL16_GRAY + i] = i;
		}
		SplTransTbl[PAL16_BEIGE + 15] = 0;
		SplTransTbl[PAL16_YELLOW + 15] = 0;
		SplTransTbl[PAL16_ORANGE + 15] = 0;
		break;
	case SpellType::Skill:
		break;
	}
}

} // namespace devilution
