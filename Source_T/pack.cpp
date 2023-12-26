/**
 * @file pack.cpp
 *
 * Implementation of functions for minifying player data structure.
 */
#include "pack.h"

#include <cstdint>

#include "engine/random.hpp"
#include "init.h"
#include "loadsave.h"
#include "playerdat.hpp"
#include "stores.h"
#include "utils/endian.hpp"

namespace devilution {

namespace {

void VerifyGoldSeeds(Player &player)
{
	for (int i = 0; i < player._pNumInv; i++) {
		if (player.InvList[i].IDidx != IDI_GOLD)
			continue;
		for (int j = 0; j < player._pNumInv; j++) {
			if (i == j)
				continue;
			if (player.InvList[j].IDidx != IDI_GOLD)
				continue;
			if (player.InvList[i]._iSeed != player.InvList[j]._iSeed)
				continue;
			player.InvList[i]._iSeed = AdvanceRndSeed();
			j = -1;
		}
	}
}

} // namespace

void PackItem(ItemPack &packedItem, const Item &item, bool isHellfire)
{
	packedItem = {};
	// Arena potions don't exist in vanilla so don't save them to stay backward compatible
	if (item.isEmpty() || item._iMiscId == IMISC_ARENAPOT) {
		packedItem.idx = 0xFFFF;
	} else {
		auto idx = item.IDidx;
		if (!isHellfire) {
			idx = RemapItemIdxToDiablo(idx);
		}
		if (gbIsSpawn) {
			idx = RemapItemIdxToSpawn(idx);
		}
		packedItem.idx = SDL_SwapLE16(idx);
		if (item.IDidx == IDI_EAR) {
			packedItem.iCreateInfo = SDL_SwapLE16(item._iIName[1] | (item._iIName[0] << 8));
			packedItem.iSeed = SDL_SwapLE32(LoadBE32(&item._iIName[2]));
			packedItem.bId = item._iIName[6];
			packedItem.bDur = item._iIName[7];
			packedItem.bMDur = item._iIName[8];
			packedItem.bCh = item._iIName[9];
			packedItem.bMCh = item._iIName[10];
			packedItem.wValue = SDL_SwapLE16(item._ivalue | (item._iIName[11] << 8) | ((item._iCurs - ICURS_EAR_SORCERER) << 6));
			packedItem.dwBuff = SDL_SwapLE32(LoadBE32(&item._iIName[12]));
		} else {
			packedItem.iSeed = SDL_SwapLE32(item._iSeed);
			packedItem.iCreateInfo = SDL_SwapLE16(item._iCreateInfo);
			packedItem.bId = (item._iMagical << 1) | (item._iIdentified ? 1 : 0);
			packedItem.bDur = item._iDurability;
			packedItem.bMDur = item._iMaxDur;
			packedItem.bCh = item._iCharges;
			packedItem.bMCh = item._iMaxCharges;
			if (item.IDidx == IDI_GOLD)
				packedItem.wValue = SDL_SwapLE16(item._ivalue);
			packedItem.dwBuff = item.dwBuff;
		}
	}
}

//void PackPlayer(PlayerPack *pPack, const Player &player, bool manashield, bool ethershield, bool hlthregn, bool manaregn, bool dmgreduct, bool netSync)
void PackPlayer(PlayerPack *pPack, const Player &player, bool manashield , bool netSync)
{
	bool dmgreduct = player.pDmgReduct;
	bool ethershield = player.pEtherShield;
	bool hlthregn = player.pHlthregn;
	bool manaregn = player.pManaregn;
	memset(pPack, 0, sizeof(*pPack));
	pPack->destAction = player.destAction;
	pPack->destParam1 = player.destParam1;
	pPack->destParam2 = player.destParam2;
	pPack->plrlevel = player.plrlevel;
	pPack->px = player.position.tile.x;
	pPack->py = player.position.tile.y;
	if (gbVanilla) {
		pPack->targx = player.position.tile.x;
		pPack->targy = player.position.tile.y;
	}
	strcpy(pPack->pName, player._pName);
	pPack->pClass = static_cast<int8_t>(player._pClass);
	pPack->pBaseStr = player._pBaseStr;
	pPack->pBaseMag = player._pBaseMag;
	pPack->pBaseDex = player._pBaseDex;
	pPack->pBaseVit = player._pBaseVit;
	pPack->pClasstype = player._pClasstype;
	pPack->pLevel = player._pLevel;
	pPack->pStatPts = player._pStatPts;
	pPack->pExperience = SDL_SwapLE32(player._pExperience);
	pPack->pGold = SDL_SwapLE32(player._pGold);
	pPack->pHPBase = SDL_SwapLE32(player._pHPBase);
	pPack->pMaxHPBase = SDL_SwapLE32(player._pMaxHPBase);
	pPack->pManaBase = SDL_SwapLE32(player._pManaBase);
	pPack->pMaxManaBase = SDL_SwapLE32(player._pMaxManaBase);
	pPack->pMemSpells = SDL_SwapLE64(player._pMemSpells);

	for (int i = 0; i < 46; i++) // Should be MAX_SPELLS but set to 37 to make save games compatible
		pPack->pSplLvl[i] = player._pSplLvl[i];
	for (int i = 46; i < 56; i++) //47 without custom spells
		pPack->pSplLvl2[i - 46] = player._pSplLvl[i];

	for (int i = 0; i < NUM_INVLOC; i++) {
		const Item &item = player.InvBody[i];
		bool isHellfire = netSync ? ((item.dwBuff & CF_HELLFIRE) != 0) : gbIsHellfire;
		PackItem(pPack->InvBody[i], item, isHellfire);
	}

	pPack->_pNumInv = player._pNumInv;
	for (int i = 0; i < pPack->_pNumInv; i++) {
		const Item &item = player.InvList[i];
		bool isHellfire = netSync ? ((item.dwBuff & CF_HELLFIRE) != 0) : gbIsHellfire;
		PackItem(pPack->InvList[i], item, isHellfire);
	}

	for (int i = 0; i < InventoryGridCells; i++)
		pPack->InvGrid[i] = player.InvGrid[i];

	for (int i = 0; i < MaxBeltItems; i++) {
		const Item &item = player.SpdList[i];
		bool isHellfire = netSync ? ((item.dwBuff & CF_HELLFIRE) != 0) : gbIsHellfire;
		PackItem(pPack->SpdList[i], item, isHellfire);
	}

	pPack->wReflections = SDL_SwapLE16(player.wReflections);
	pPack->pDifficulty = SDL_SwapLE32(player.pDifficulty);
	pPack->pDamAcFlags = SDL_SwapLE32(static_cast<uint32_t>(player.pDamAcFlags));
	pPack->pDiabloKillLevel = SDL_SwapLE32(player.pDiabloKillLevel);
	pPack->bIsHellfire = gbIsHellfire ? 1 : 0;

	if (!gbIsMultiplayer || manashield)
		pPack->pManaShield = SDL_SwapLE32(player.pManaShield);
	else
		pPack->pManaShield = 0;

	if (!gbIsMultiplayer || dmgreduct)
		pPack->pDmgReduct = SDL_SwapLE32(player.pDmgReduct);
	else
		pPack->pDmgReduct = 0;

	if (!gbIsMultiplayer || ethershield)
		pPack->pEtherShield = SDL_SwapLE32(player.pEtherShield);
	else
		pPack->pEtherShield = 0;

	if (!gbIsMultiplayer || hlthregn)
		pPack->pHlthregn = SDL_SwapLE32(player.pHlthregn);
	else
		pPack->pHlthregn = 0;

	if (!gbIsMultiplayer || manaregn)
		pPack->pManaregn = SDL_SwapLE32(player.pManaregn);
	else
		pPack->pManaregn = 0;
	//if (!gbIsMultiplayer || aurashield)
	//	pPack->pAuraShield = SDL_SwapLE32(player.pAuraShield);
	//else
	//	pPack->pAuraShield = 0;
}

void UnPackItem(const ItemPack &packedItem, const Player &player, Item &item, bool isHellfire)
{
	auto idx = static_cast<_item_indexes>(SDL_SwapLE16(packedItem.idx));

	if (gbIsSpawn) {
		idx = RemapItemIdxFromSpawn(idx);
	}
	if (!isHellfire) {
		idx = RemapItemIdxFromDiablo(idx);
	}

	if (!IsItemAvailable(idx)) {
		item.clear();
		return;
	}

	if (idx == IDI_EAR) {
		uint16_t ic = SDL_SwapLE16(packedItem.iCreateInfo);
		uint32_t iseed = SDL_SwapLE32(packedItem.iSeed);
		uint16_t ivalue = SDL_SwapLE16(packedItem.wValue);
		int32_t ibuff = SDL_SwapLE32(packedItem.dwBuff);

		char heroName[17];
		heroName[0] = static_cast<char>((ic >> 8) & 0x7F);
		heroName[1] = static_cast<char>(ic & 0x7F);
		heroName[2] = static_cast<char>((iseed >> 24) & 0x7F);
		heroName[3] = static_cast<char>((iseed >> 16) & 0x7F);
		heroName[4] = static_cast<char>((iseed >> 8) & 0x7F);
		heroName[5] = static_cast<char>(iseed & 0x7F);
		heroName[6] = static_cast<char>(packedItem.bId & 0x7F);
		heroName[7] = static_cast<char>(packedItem.bDur & 0x7F);
		heroName[8] = static_cast<char>(packedItem.bMDur & 0x7F);
		heroName[9] = static_cast<char>(packedItem.bCh & 0x7F);
		heroName[10] = static_cast<char>(packedItem.bMCh & 0x7F);
		heroName[11] = static_cast<char>((ivalue >> 8) & 0x7F);
		heroName[12] = static_cast<char>((ibuff >> 24) & 0x7F);
		heroName[13] = static_cast<char>((ibuff >> 16) & 0x7F);
		heroName[14] = static_cast<char>((ibuff >> 8) & 0x7F);
		heroName[15] = static_cast<char>(ibuff & 0x7F);
		heroName[16] = '\0';

		RecreateEar(item, ic, iseed, ivalue & 0xFF, heroName);
	} else {
		item = {};
		RecreateItem(player, item, idx, SDL_SwapLE16(packedItem.iCreateInfo), SDL_SwapLE32(packedItem.iSeed), SDL_SwapLE16(packedItem.wValue), isHellfire);
		item._iMagical = static_cast<item_quality>(packedItem.bId >> 1);
		item._iIdentified = (packedItem.bId & 1) != 0;
		item._iDurability = packedItem.bDur;
		item._iMaxDur = packedItem.bMDur;
		item._iCharges = packedItem.bCh;
		item._iMaxCharges = packedItem.bMCh;

		RemoveInvalidItem(item);

		if (isHellfire)
			item.dwBuff |= CF_HELLFIRE;
		else
			item.dwBuff &= ~CF_HELLFIRE;
	}
}

bool UnPackPlayer(const PlayerPack *pPack, Player &player, bool netSync)
{
	Point position { pPack->px, pPack->py };
	if (!InDungeonBounds(position)) {
		return false;
	}

	uint8_t dungeonLevel = pPack->plrlevel;
	if (dungeonLevel >= NUMLEVELS) {
		return false;
	}

	if (pPack->pClass >= enum_size<HeroClass>::value) {
		return false;
	}
	auto heroClass = static_cast<HeroClass>(pPack->pClass);

	if (pPack->pLevel > MaxCharacterLevel || pPack->pLevel < 1) {
		return false;
	}
	uint32_t difficulty = SDL_SwapLE32(pPack->pDifficulty);
	if (difficulty > DIFF_LAST) {
		return false;
	}

	player._pLevel = pPack->pLevel;

	player.position.tile = position;
	player.position.future = position;
	player.setLevel(dungeonLevel);

	player._pClass = heroClass;

	ClrPlrPath(player);
	player.destAction = ACTION_NONE;

	strcpy(player._pName, pPack->pName);

	InitPlayer(player, true);

	player._pBaseStr = pPack->pBaseStr;
	player._pStrength = pPack->pBaseStr;
	player._pBaseMag = pPack->pBaseMag;
	player._pMagic = pPack->pBaseMag;
	player._pBaseDex = pPack->pBaseDex;
	player._pDexterity = pPack->pBaseDex;
	player._pBaseVit = pPack->pBaseVit;
	player._pVitality = pPack->pBaseVit;
	player._pClasstype = pPack->pClasstype;


	player._pStatPts = pPack->pStatPts;
	player._pExperience = SDL_SwapLE32(pPack->pExperience);
	player._pGold = SDL_SwapLE32(pPack->pGold);
	player._pMaxHPBase = SDL_SwapLE32(pPack->pMaxHPBase);
	player._pHPBase = SDL_SwapLE32(pPack->pHPBase);
	player._pBaseToBlk = PlayersData[static_cast<std::size_t>(player._pClass)].blockBonus;
	if (!netSync)
		if ((int)(player._pHPBase & 0xFFFFFFC0) < 64)
			player._pHPBase = 64;

	player._pMaxManaBase = SDL_SwapLE32(pPack->pMaxManaBase);
	player._pManaBase = SDL_SwapLE32(pPack->pManaBase);
	player._pMemSpells = SDL_SwapLE64(pPack->pMemSpells);

	for (int i = 0; i < 46; i++) // Should be MAX_SPELLS but set to 37 to make save games compatible
		player._pSplLvl[i] = pPack->pSplLvl[i];
	for (int i = 46; i < 56; i++) //47 original
		player._pSplLvl[i] = pPack->pSplLvl2[i - 46];

	for (int i = 0; i < NUM_INVLOC; i++) {
		auto packedItem = pPack->InvBody[i];
		bool isHellfire = netSync ? ((packedItem.dwBuff & CF_HELLFIRE) != 0) : (pPack->bIsHellfire != 0);
		UnPackItem(packedItem, player, player.InvBody[i], isHellfire);
	}

	player._pNumInv = pPack->_pNumInv;
	for (int i = 0; i < player._pNumInv; i++) {
		auto packedItem = pPack->InvList[i];
		bool isHellfire = netSync ? ((packedItem.dwBuff & CF_HELLFIRE) != 0) : (pPack->bIsHellfire != 0);
		UnPackItem(packedItem, player, player.InvList[i], isHellfire);
	}

	for (int i = 0; i < InventoryGridCells; i++)
		player.InvGrid[i] = pPack->InvGrid[i];

	VerifyGoldSeeds(player);

	for (int i = 0; i < MaxBeltItems; i++) {
		auto packedItem = pPack->SpdList[i];
		bool isHellfire = netSync ? ((packedItem.dwBuff & CF_HELLFIRE) != 0) : (pPack->bIsHellfire != 0);
		UnPackItem(packedItem, player, player.SpdList[i], isHellfire);
	}

	CalcPlrInv(player, false);
	player.wReflections = SDL_SwapLE16(pPack->wReflections);
	player.pTownWarps = 0;
	player.pDungMsgs = 0;
	player.pDungMsgs2 = 0;
	player.pLvlLoad = 0;
	player.pDiabloKillLevel = SDL_SwapLE32(pPack->pDiabloKillLevel);
	player.pBattleNet = pPack->pBattleNet != 0;
	player.pManaShield = pPack->pManaShield != 0;
	player.pDmgReduct = pPack->pDmgReduct != 0;
	player.pEtherShield = pPack->pEtherShield != 0;
	player.pHlthregn = pPack->pHlthregn != 0;
	player.pManaregn = pPack->pManaregn != 0;
	//player.pAuraShield = pPack->pAuraShield != 0;
	player.pDifficulty = static_cast<_difficulty>(difficulty);
	player.pDamAcFlags = static_cast<ItemSpecialEffectHf>(SDL_SwapLE32(static_cast<uint32_t>(pPack->pDamAcFlags)));

	return true;
}

} // namespace devilution
