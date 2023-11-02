/**
 * @file spells.cpp
 *
 * Implementation of functionality for casting player spells.
 */
#include "spells.h"

#include "control.h"
#include "cursor.h"
#ifdef _DEBUG
#include "debug.h"
#endif
#include "engine/backbuffer_state.hpp"
#include "engine/point.hpp"
#include "engine/random.hpp"
#include "gamemenu.h"
#include "inv.h"
#include "missiles.h"

namespace devilution {

namespace {

/**
 * @brief Gets a value indicating whether the player's current readied spell is a valid spell. Readied spells can be
 * invalidaded in a few scenarios where the spell comes from items, for example (like dropping the only scroll that
 * provided the spell).
 * @param player The player whose readied spell is to be checked.
 * @return 'true' when the readied spell is currently valid, and 'false' otherwise.
 */
bool IsReadiedSpellValid(const Player &player)
{
	switch (player._pRSplType) {
	case SpellType::Skill:
	case SpellType::Spell:
	case SpellType::Invalid:
		return true;

	case SpellType::Charges:
		return (player._pISpells & GetSpellBitmask(player._pRSpell)) != 0;

	case SpellType::Scroll:
		return (player._pScrlSpells & GetSpellBitmask(player._pRSpell)) != 0;

	default:
		return false;
	}
}

/**
 * @brief Clears the current player's readied spell selection.
 * @note Will force a UI redraw in case the values actually change, so that the new spell reflects on the bottom panel.
 * @param player The player whose readied spell is to be cleared.
 */
void ClearReadiedSpell(Player &player)
{
	if (player._pRSpell != SpellID::Invalid) {
		player._pRSpell = SpellID::Invalid;
		RedrawEverything();
	}

	if (player._pRSplType != SpellType::Invalid) {
		player._pRSplType = SpellType::Invalid;
		RedrawEverything();
	}
}

} // namespace

bool IsValidSpell(SpellID spl)
{
	return spl > SpellID::Null
	    && spl <= SpellID::LAST
	    && (spl <= SpellID::LastDiablo || gbIsHellfire);
}

bool IsValidSpellFrom(int spellFrom)
{
	if (spellFrom == 0)
		return true;
	if (spellFrom >= INVITEM_INV_FIRST && spellFrom <= INVITEM_INV_LAST)
		return true;
	if (spellFrom >= INVITEM_BELT_FIRST && spellFrom <= INVITEM_BELT_LAST)
		return true;
	return false;
}

bool IsWallSpell(SpellID spl)
{
	return spl == SpellID::FireWall || spl == SpellID::LightningWall;
}

bool TargetsMonster(SpellID id)
{
	return id == SpellID::Fireball
	    || id == SpellID::FireWall
	    || id == SpellID::Inferno
	    || id == SpellID::Lightning
	    || id == SpellID::StoneCurse
	    || id == SpellID::FlameWave;
}

int GetManaAmount(const Player &player, SpellID sn)
{
	int ma; // mana amount

	// mana adjust
	int adj = 0;

	// spell level
	int sl = std::max(player.GetSpellLevel(sn) - 1, 0);

	if (player.pEtherShield)
	{ // Blood Magic
		if (sl > 0) {
			adj = 0;
		}
		if (sn == SpellID::Apocalypse) {
			adj = 0;
		}
		if (sn == SpellID::BloodStar) {
			adj = 0;
		}
		if (sn == SpellID::ChainLightning) {
			adj = 0;
		}
		if (sn == SpellID::ChargedBolt) {
			adj = 0;
		}
		if (sn == SpellID::DoomSerpents) {
			adj = 0;
		}
		if (sn == SpellID::Elemental) {
			adj = 0;
		}
		if (sn == SpellID::Etherealize) {
			adj = 0;
		}
		if (sn == SpellID::Fireball) {
			adj = 0;
		}
		if (sn == SpellID::Firebolt) {
			adj = 0;
		}
		if (sn == SpellID::FireWall) {
			adj = 0;
		}
		if (sn == SpellID::FlameWave) {
			adj = 0;
		}
		if (sn == SpellID::Flash) {
			adj = 0;
		}
		if (sn == SpellID::Golem) {
			adj = 0;
		}
		if (sn == SpellID::Guardian) {
			adj = 0;
		}
		if (sn == SpellID::HealOther) {
			adj = 0;
		}
		if (sn == SpellID::HolyBolt) {
			adj = 0;
		}
		if (sn == SpellID::Identify) {
			adj = 0;
		}
		if (sn == SpellID::Immolation) {
			adj = 0;
		}
		if (sn == SpellID::Inferno) {
			adj = 0;
		}
		if (sn == SpellID::Infravision) {
			adj = 0;
		}
		if (sn == SpellID::ItemRepair) {
			adj = 0;
		}
		if (sn == SpellID::Jester) {
			adj = 0;
		}
		if (sn == SpellID::Lightning) {
			adj = 0;
		}
		if (sn == SpellID::LightningWall) {
			adj = 0;
		}
		if (sn == SpellID::ManaRegen) {
			adj = 0;
		}
		if (sn == SpellID::DmgReduct) {
			adj = 0;
		}
		if (sn == SpellID::BloodM) {
			goto nobloodm;
		}
		if (sn == SpellID::HealthRegen) {
			adj = 0;
		}
		if (sn == SpellID::Healing) {
			goto nobloodm;
		}
		if (sn == SpellID::ManaShield) {
			goto nobloodm;
		}
		if (sn == SpellID::Nova) {
			adj = 0;
		}
		if (sn == SpellID::Phasing) {
			adj = 0;
		}
		if (sn == SpellID::Reflect) {
			adj = 0;
		}
		if (sn == SpellID::Resurrect) {
			adj = 0;
		}
		if (sn == SpellID::RingOfFire) {
			adj = 0;
		}
		if (sn == SpellID::Rage) {
			adj = 0;
		}
		if (sn == SpellID::Search) {
			adj = 0;
		}
		if (sn == SpellID::Smite) {
			adj = 0;
		}
		if (sn == SpellID::StaffRecharge) {
			adj = 0;
		}
		if (sn == SpellID::StoneCurse) {
			adj = 0;
		}
		if (sn == SpellID::Telekinesis) {
			adj = 0;
		}
		if (sn == SpellID::Teleport) {
			adj = 0;
		}
		if (sn == SpellID::TownPortal) {
			adj = 0;
		}
		if (sn == SpellID::TrapDisarm) {
			adj = 0;
		}
		if (sn == SpellID::Warp) {
			adj = 0;
		}

		if (sn == SpellID::Healing || sn == SpellID::HealOther) {
			ma = 0;
		} else if ((GetSpellData(sn).sManaCost) == 255) {
			ma = 0;
		} else {
			ma = 0;
		}

		ma = std::max(ma, 1);
		ma <<= 1;

		/*if (gbIsHellfire && player._pClass == HeroClass::Sorcerer) {
		    ma /= 2 * 2;
		} else if (player._pClass == HeroClass::Rogue || player._pClass == HeroClass::Monk || player._pClass == HeroClass::Bard) {
		    ma -= ma / 4 * 2;
		}*/

		if (GetSpellData(sn).sMinMana > ma >> 1) {
			ma = GetSpellData(sn).sMinMana << 1;
		}
		goto xtblbm;
	}

	nobloodm:;

	if (player._pClass == HeroClass::Warlock) {
		if (sl > 0) {
			adj = sl * (GetSpellData(sn).sManaAdj * 2);
		}
		if (sn == SpellID::Firebolt) {
			adj /= 2 * 2;
		}
		if (sn == SpellID::Resurrect && sl > 0) {
			adj = sl * ((GetSpellData(SpellID::Resurrect).sManaCost * 2) / 8);
		}

		if (sn == SpellID::Healing || sn == SpellID::HealOther) {
			ma = ((GetSpellData(SpellID::Healing).sManaCost * 2) + 2 * (player._pLevel / 2) - adj);
		} else if ((GetSpellData(sn).sManaCost * 2) == 255) {
			ma = (player._pMaxManaBase >> 6) - adj;
		} else {
			ma = ((GetSpellData(sn).sManaCost * 2) - adj);
		}

		ma = std::max(ma, 0);
		ma <<= 6;

		if (GetSpellData(sn).sMinMana > ma >> 6) {
			ma = GetSpellData(sn).sMinMana << 6;
		}
	} else if (player._pClass == HeroClass::Bard) {
		if (sl > 0) {
			adj = sl * (GetSpellData(sn).sManaAdj / 2);
		}
		if (sn == SpellID::Firebolt) {
			adj /= 2 / 2;
		}
		if (sn == SpellID::HealthRegen) {
			adj = 0;
		}
		if (sn == SpellID::Berserk) {
			adj = 0;
		}
		if (sn == SpellID::Resurrect && sl > 0) {
			adj = sl * ((GetSpellData(SpellID::Resurrect).sManaCost / 2) / 8);
		}

		if (sn == SpellID::Healing || sn == SpellID::HealOther) {
			ma = ((GetSpellData(SpellID::Healing).sManaCost / 2) + 2 * (player._pLevel / 2) - adj);
		} else if ((GetSpellData(sn).sManaCost / 2) == 255) {
			ma = (player._pMaxManaBase >> 6) - adj;
		} else {
			ma = ((GetSpellData(sn).sManaCost / 2) - adj);
		}

		ma = std::max(ma, 0);
		ma <<= 6;

		/*if (gbIsHellfire && player._pClass == HeroClass::Sorcerer) {
		    ma /= 2 * 2;
		} else if (player._pClass == HeroClass::Rogue || player._pClass == HeroClass::Monk || player._pClass == HeroClass::Bard) {
		    ma -= ma / 4 * 2;
		}*/

		if (GetSpellData(sn).sMinMana > ma >> 6) {
			ma = GetSpellData(sn).sMinMana << 6;
		}
	} else if (player._pClass == HeroClass::Barbarian) {
		if (sl > 0) {
			adj = sl * (GetSpellData(sn).sManaAdj / 2);
		}
		if (sn == SpellID::Firebolt) {
			adj /= 2 / 2;
		}
		if (sn == SpellID::Rage) {
			adj = 0;
		}
		if (sn == SpellID::Resurrect && sl > 0) {
			adj = sl * ((GetSpellData(SpellID::Resurrect).sManaCost / 2) / 8);
		}

		if (sn == SpellID::Healing || sn == SpellID::HealOther) {
			ma = ((GetSpellData(SpellID::Healing).sManaCost / 2) + 2 * (player._pLevel / 2) - adj);
		} else if ((GetSpellData(sn).sManaCost / 2) == 255) {
			ma = (player._pMaxManaBase >> 6) - adj;
		} else {
			ma = ((GetSpellData(sn).sManaCost / 2) - adj);
		}

		ma = std::max(ma, 0);
		ma <<= 6;

		/*if (gbIsHellfire && player._pClass == HeroClass::Sorcerer) {
		    ma /= 2 * 2;
		} else if (player._pClass == HeroClass::Rogue || player._pClass == HeroClass::Monk || player._pClass == HeroClass::Bard) {
		    ma -= ma / 4 * 2;
		}*/

		if (GetSpellData(sn).sMinMana > ma >> 6) {
			ma = GetSpellData(sn).sMinMana << 6;
		}
	} else if (player._pClass == HeroClass::Assassin) {
		//if ((player._pMana >> 6) > 0) {
			if (sl > 0) {
				adj = player._pManaPer;
			}
			if (sn == SpellID::Firebolt) {
				adj = player._pManaPer;
			}
			if (sn == SpellID::Resurrect && sl > 0) {
				adj = player._pManaPer;
			}

			if (sn == SpellID::Healing || sn == SpellID::HealOther) {
				ma = player._pManaPer;
			} else if ((GetSpellData(sn).sManaCost / 2) == 255) {
				ma = player._pManaPer;
			} else {
				ma = player._pManaPer;
		    }
		    if (sn == SpellID::Teleport) {
			    ma = 0;
		    }

			ma = std::max(ma, 0);
			ma <<= 6;

			if (GetSpellData(sn).sMinMana > ma >> 6) {
				ma = player._pManaPer;
			}
	} else if (player._pClass == HeroClass::Witch) {
		if ((player._pMana >> 6) > 0) {
			if (sl > 0) {
				adj = player._pManaPer;
			}
			if (sn == SpellID::Firebolt) {
				adj = player._pManaPer;
			}
			if (sn == SpellID::Resurrect && sl > 0) {
				adj = player._pManaPer;
			}

			if (sn == SpellID::Healing || sn == SpellID::HealOther) {
				ma = player._pManaPer;
			} else if ((GetSpellData(sn).sManaCost / 2) == 255) {
				ma = player._pManaPer;
			} else {
				ma = player._pManaPer;
			}

			ma = std::max(ma, 0);
			ma <<= 6;

			if (GetSpellData(sn).sMinMana > ma >> 6) {
				ma = player._pManaPer;
			}
		} else {
			if (sl > 0) {
				adj = player._pManaPer + 1;
			}
			if (sn == SpellID::Firebolt) {
				adj = player._pManaPer + 1;
			}
			if (sn == SpellID::Resurrect && sl > 0) {
				adj = player._pManaPer + 1;
			}

			if (sn == SpellID::Healing || sn == SpellID::HealOther) {
				ma = player._pManaPer + 1;
			} else if ((GetSpellData(sn).sManaCost / 2) == 255) {
				ma = player._pManaPer + 1;
			} else {
				ma = player._pManaPer + 1;
			}

			ma = std::max(ma, 0);
			ma <<= 6;

			if (GetSpellData(sn).sMinMana > ma >> 6) {
				ma = player._pManaPer + 1;
			}
		}
	} else if (player._pClass == HeroClass::Sorcerer || player._pClass == HeroClass::Sage || player._pClass == HeroClass::Bloodmage || player._pClass == HeroClass::Battlemage || player._pClass == HeroClass::Traveler) {
		if (sl > 0) {
			adj = sl * (GetSpellData(sn).sManaAdj);
		}
		if (sn == SpellID::Firebolt) {
			adj /= 2;
		}
		if (sn == SpellID::Resurrect && sl > 0) {
			adj = sl * ((GetSpellData(SpellID::Resurrect).sManaCost) / 8);
		}

		if (sn == SpellID::Healing || sn == SpellID::HealOther) {
			ma = ((GetSpellData(SpellID::Healing).sManaCost) + 2 * (player._pLevel / 2) - adj);
		} else if ((GetSpellData(sn).sManaCost) == 255) {
			ma = (player._pMaxManaBase >> 6) - adj;
		} else {
			ma = ((GetSpellData(sn).sManaCost) - adj);
		}

		ma = std::max(ma, 0);
		ma <<= 6;

		/*if (gbIsHellfire && player._pClass == HeroClass::Sorcerer) {
			ma /= 2 * 2;
		} else if (player._pClass == HeroClass::Rogue || player._pClass == HeroClass::Monk || player._pClass == HeroClass::Bard) {
			ma -= ma / 4 * 2;
		}*/

		if (GetSpellData(sn).sMinMana > ma >> 6) {
			ma = GetSpellData(sn).sMinMana << 6;
		}
	}  else {
		if (sl > 0) {
			adj = sl * GetSpellData(sn).sManaAdj;
		}
		if (sn == SpellID::Firebolt) {
			adj /= 2;
		}
		if (sn == SpellID::Resurrect && sl > 0) {
			adj = sl * (GetSpellData(SpellID::Resurrect).sManaCost / 8);
		}

		if (sn == SpellID::Healing || sn == SpellID::HealOther) {
			ma = (GetSpellData(SpellID::Healing).sManaCost + 2 * (player._pLevel / 2) - adj);
		} else if (GetSpellData(sn).sManaCost == 255) {
			ma = (player._pMaxManaBase >> 6) - adj;
		} else {
			ma = (GetSpellData(sn).sManaCost - adj);
		}

		ma = std::max(ma, 0);
		ma <<= 6;

		/*if (gbIsHellfire && player._pClass == HeroClass::Sorcerer) {
			ma /= 2;
		} else if (player._pClass == HeroClass::Rogue || player._pClass == HeroClass::Monk || player._pClass == HeroClass::Bard) {
			ma -= ma / 4;
		}*/

		if (GetSpellData(sn).sMinMana > ma >> 6) {
			ma = GetSpellData(sn).sMinMana << 6;
		}
	}
xtblbm:;

	return ma;
}

void ConsumeSpell(Player &player, SpellID sn)
{
	int hpremoved = (player._pLevel / 2);
	int mpremoved = (player._pLevel * 2);

	switch (player.executedSpell.spellType) {
	case SpellType::Skill:
	case SpellType::Invalid:
		break;
	case SpellType::Scroll:
		ConsumeScroll(player);
		break;
	case SpellType::Charges:
		ConsumeStaffCharge(player);
		break;
	case SpellType::Spell:
//#ifdef _DEBUG
//		if (DebugGodMode)
//			break;
//#endif
		int ma = GetManaAmount(player, sn);
		player._pMana -= ma;
		player._pManaBase -= ma;
		/*int hpremoved = (player._pLevel / 2);
		int mpremoved = (player._pLevel * 2);
		if (sn == SpellID::Magi) {
			ApplyPlrDamage(DamageType::Physical, player, hpremoved);
		}*/

		if (player._pClasstype == 1 || player._pClasstype == 2 || player._pClasstype == 3 || player._pClasstype == 4 || player._pClasstype == 7 || player._pClasstype == 9 || player._pClasstype == 10 || player._pClasstype == 11 || player._pClasstype == 12 || player._pClasstype == 13 || player._pClasstype == 14 || player._pClasstype == 15 || player._pClasstype == 16) 
			RedrawComponent(PanelDrawComponent::Mana);
		break;
	}
		if (sn == SpellID::Magi) {
			ApplyPlrDamage(DamageType::Physical, player, hpremoved);
		}
	if (sn == SpellID::WitchBS) {
		ApplyPlrDamage(DamageType::Physical, player, 5);
	}
	if (sn == SpellID::BoneSpirit) {
		ApplyPlrDamage(DamageType::Physical, player, 6);
	}
	if (player.pEtherShield) {

		int ma; // mana amount

		// mana adjust
		int adj = 0;

		// spell level
		int sl = std::max(player.GetSpellLevel(sn) - 1, 0);
		if (sl > 0) {
		adj = sl * GetSpellData(sn).sManaAdj;
		}
		if (sn == SpellID::Firebolt) {
		adj /= 2;
		}
		if (sn == SpellID::Resurrect && sl > 0) {
		adj = sl * (GetSpellData(SpellID::Resurrect).sManaCost / 8);
		}

		if (sn == SpellID::Healing || sn == SpellID::HealOther) {
		ma = (GetSpellData(SpellID::Healing).sManaCost + 2 * (player._pLevel / 2) - adj);
		} else if (GetSpellData(sn).sManaCost == 255) {
		ma = (player._pMaxManaBase >> 6) - adj;
		} else {
		ma = (GetSpellData(sn).sManaCost - adj);
		}
		ma = std::max(ma, 0);
		ma <<= 6;

		if (GetSpellData(sn).sMinMana > ma >> 6) {
		ma = GetSpellData(sn).sMinMana << 6;
		}

		int blood;
		blood = ma / 100;


		if (sn == SpellID::Apocalypse) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::BloodStar) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::ChainLightning) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::ChargedBolt) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::DoomSerpents) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Elemental) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Etherealize) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Fireball) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Firebolt) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::FireWall) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::FlameWave) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Flash) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Golem) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Guardian) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::HealOther) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::HolyBolt) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Identify) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Immolation) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Inferno) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Infravision) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::ItemRepair) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Jester) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Lightning) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::LightningWall) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::ManaRegen) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::DmgReduct) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		//if (sn == SpellID::BloodM) {
		//ApplyPlrDamage(DamageType::Physical, player, blood);
		//}
		if (sn == SpellID::HealthRegen) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		//if (sn == SpellID::Healing) {
		//ApplyPlrDamage(DamageType::Physical, player, blood);
		//}
		//if (sn == SpellID::ManaShield) {
		//ApplyPlrDamage(DamageType::Physical, player, blood);
		//}
		if (sn == SpellID::Nova) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Phasing) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Reflect) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Resurrect) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::RingOfFire) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Rage) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Search) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Smite) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::StaffRecharge) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::StoneCurse) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Telekinesis) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Teleport) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::TownPortal) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::TrapDisarm) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		if (sn == SpellID::Warp) {
		ApplyPlrDamage(DamageType::Physical, player, blood);
		}
		//if (sn == SpellID::WitchBS) {
		//ApplyPlrDamage(DamageType::Physical, player, blood);
		//}
		//if (sn == SpellID::) {
		//ApplyPlrDamage(DamageType::Physical, player, 6);
		//}
		RedrawComponent(PanelDrawComponent::Health);
	}
	//int hp = player._pHitPoints;
	//int hpremoved = hp / 10;
	//// old for skilll version of magi


	//if (sn == SpellID::Berserk) {
		//player._pMana -= mpremoved;
		//player._pManaBase -= mpremoved;
	//	if (player._pClasstype != 5 || 8 || 13)
		//RedrawComponent(PanelDrawComponent::Mana);
	//	ApplyPlrDamage(DamageType::Physical, player, hpremoved);
	//}
}

void EnsureValidReadiedSpell(Player &player)
{
	if (!IsReadiedSpellValid(player)) {
		ClearReadiedSpell(player);
	}
}

SpellCheckResult CheckSpell(const Player &player, SpellID sn, SpellType st, bool manaonly)
{
#ifdef _DEBUG
	if (DebugGodMode)
		return SpellCheckResult::Success;
#endif

	if (!manaonly && pcurs != CURSOR_HAND) {
		return SpellCheckResult::Fail_Busy;
	}

	if (st == SpellType::Skill) {
		return SpellCheckResult::Success;
	}

	if (player.GetSpellLevel(sn) <= 0) {
		return SpellCheckResult::Fail_Level0;
	}

	if (player.pEtherShield) {
	//if (player._pClass == HeroClass::Bloodmage) {
		if (sn == SpellID::Healing || sn == SpellID::HealOther || sn == SpellID::ManaShield || sn == SpellID::BloodM || sn == SpellID::WitchBS || sn == SpellID::BoneSpirit || sn == SpellID::Magi) {
		goto NotBM;
		}
		if (player._pHitPoints < GetManaAmount(player, sn)) {
		return SpellCheckResult::Fail_NoMana;
		}
		goto extBM;
	}
NotBM:;
	if (player._pMana < GetManaAmount(player, sn)) {
		return SpellCheckResult::Fail_NoMana;
	}
	extBM:;
	return SpellCheckResult::Success;
}

void CastSpell(int id, SpellID spl, int sx, int sy, int dx, int dy, int spllvl)
{
	Player &player = Players[id];
	Direction dir = player._pdir;
	if (IsWallSpell(spl)) {
		dir = player.tempDirection;
	}

	bool fizzled = false;
	const SpellData &spellData = GetSpellData(spl);
	for (size_t i = 0; i < sizeof(spellData.sMissiles) / sizeof(spellData.sMissiles[0]) && spellData.sMissiles[i] != MissileID::Null; i++) {
		Missile *missile = AddMissile({ sx, sy }, { dx, dy }, dir, spellData.sMissiles[i], TARGET_MONSTERS, id, 0, spllvl);
		fizzled |= (missile == nullptr);
	}
	if (spl == SpellID::ChargedBolt) {
		for (int i = (spllvl / 2) + 3; i > 0; i--) {
			Missile *missile = AddMissile({ sx, sy }, { dx, dy }, dir, MissileID::ChargedBolt, TARGET_MONSTERS, id, 0, spllvl);
			fizzled |= (missile == nullptr);
		}
	}
	if (!fizzled) {
		ConsumeSpell(player, spl);
	}
}

void DoResurrect(size_t pnum, Player &target)
{
	if (pnum >= Players.size()) {
		return;
	}

	AddMissile(target.position.tile, target.position.tile, Direction::South, MissileID::ResurrectBeam, TARGET_MONSTERS, pnum, 0, 0);

	if (target._pHitPoints != 0)
		return;

	if (&target == MyPlayer) {
		MyPlayerIsDead = false;
		gamemenu_off();
		RedrawComponent(PanelDrawComponent::Health);
		//if (player._pClasstype != 5 || 8 || 13)
		RedrawComponent(PanelDrawComponent::Mana);
	}

	ClrPlrPath(target);
	target.destAction = ACTION_NONE;
	target._pInvincible = false;
	SyncInitPlrPos(target);

	int hp = 10 << 6;
	if (target._pMaxHPBase < (10 << 6)) {
		hp = target._pMaxHPBase;
	}
	SetPlayerHitPoints(target, hp);

	target._pHPBase = target._pHitPoints + (target._pMaxHPBase - target._pMaxHP); // CODEFIX: does the same stuff as SetPlayerHitPoints above, can be removed
	target._pMana = 0;
	target._pManaBase = target._pMana + (target._pMaxManaBase - target._pMaxMana);

	target._pmode = PM_STAND;

	CalcPlrInv(target, true);

	if (target.isOnActiveLevel()) {
		StartStand(target, target._pdir);
	}
}

void DoHealOther(const Player &caster, Player &target)
{
	if ((target._pHitPoints >> 6) <= 0) {
		return;
	}

	int hp = (GenerateRnd(10) + 1) << 6;
	for (int i = 0; i < (caster._pLevel / 2); i++) {
		hp += (GenerateRnd(4) + 1) << 6;
	}
	for (int i = 0; i < caster.GetSpellLevel(SpellID::HealOther); i++) {
		hp += (GenerateRnd(6) + 1) << 6;
	}

	if (caster._pClass == HeroClass::Warrior || caster._pClass == HeroClass::Barbarian) {
		hp *= 2;
	} else if (caster._pClass == HeroClass::Rogue || caster._pClass == HeroClass::Bard) {
		hp += hp / 2;
	} else if (caster._pClass == HeroClass::Monk) {
		hp *= 3;
	}

	target._pHitPoints = std::min(target._pHitPoints + hp, target._pMaxHP);
	target._pHPBase = std::min(target._pHPBase + hp, target._pMaxHPBase);

	if (&target == MyPlayer) {
		RedrawComponent(PanelDrawComponent::Health);
	}
}

int GetSpellBookLevel(SpellID s)
{
	if (gbIsSpawn) {
		switch (s) {
		case SpellID::StoneCurse:
		case SpellID::Guardian:
		case SpellID::Golem:
		case SpellID::Elemental:
		case SpellID::BloodStar:
		case SpellID::BoneSpirit:
			return -1;
		default:
			break;
		}
	}

	if (!gbIsHellfire) {
		switch (s) {
		case SpellID::Nova:
		case SpellID::Apocalypse:
			return -1;
		default:
			if (s > SpellID::LastDiablo)
				return -1;
			break;
		}
	}

	return GetSpellData(s).sBookLvl;
}

int GetSpellStaffLevel(SpellID s)
{
	if (gbIsSpawn) {
		switch (s) {
		case SpellID::StoneCurse:
		case SpellID::Guardian:
		case SpellID::Golem:
		case SpellID::Apocalypse:
		case SpellID::Elemental:
		case SpellID::BloodStar:
		case SpellID::BoneSpirit:
			return -1;
		default:
			break;
		}
	}

	if (!gbIsHellfire && s > SpellID::LastDiablo)
		return -1;

	return GetSpellData(s).sStaffLvl;
}

} // namespace devilution
