/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "Hud.h"
#include "Radar.h"
#include "Pickups.h"
#include "Lists.h" // for PlayerInfo.h which uses CPtrList
#include "PlayerInfo.h"
#include "PlayerPed.h"
#include "Frontend.h"
#include "ModelIndices.h"

#include "multiplayer/natives/public.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/elements/sRadarBlip.h"
#include "multiplayer/MultiGame.h"


int mp_lsn_RemoveRadarBlip(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	int32* pHandleID = (int32*)luaL_checkudata(L, 1, "radarblip");
	if (!pHandleID || *pHandleID == -1) return 0;
	sRadarBlip* pRadar = (sRadarBlip*)Game.GetEntityForHandle(Game.m_pNetSession->m_nSelfPeerID, *pHandleID);
	if (pRadar) delete pRadar;
	*pHandleID = -1;
	return 0;
}

static const luaL_reg ls_radar_blip[] = {
	{"Remove", mp_lsn_RemoveRadarBlip},
	{"__gc",   mp_lsn_RemoveRadarBlip},
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_radar_blip, (2 + 1), (2 + 1));


void lsc_registerRadarBlip(lua_State* L, int32 nHandle) {
	int32* pHandle = (int32*)lua_newuserdata(L, sizeof(int32));
	*pHandle = nHandle;
	luaL_getmetatable(L, "radarblip");
	lua_setmetatable(L, -2);
}

/* leftover */
int mp_lsn_AddSpriteBlipForContactPoint(lua_State* L) {
	cMultiGame& Game = TheMPGame;
#ifdef GTA_LIBERTY
	int32 nPeerID = lsc_pop_peer_id_from_stack(L, 1, -1);
	if (!lua_istable(L, 1)) return 0;
#else
	int32 nPeerID = lua_tonumber(L, 1);
#endif
	CVector vPos;
	lsc_getVectorFromStack(vPos, L, 1, true);
	int32 nUnk = luaL_checknumber(L, 2); // useless
	int32 nBlipIndex = TheRadar->SetCoordBlip(eBlipType::BLIP_CONTACT_POINT, vPos, eBlipColour::RADAR_TRACE_MAGENTA, eBlipDisplay::BLIP_DISPLAY_BOTH);
	TheRadar->ChangeBlipScale(nBlipIndex, 3);
	sRadarBlip* pBlip = new sRadarBlip(nBlipIndex);
	cInterestZone* pZone = Game.m_ZoneManager.GetZoneByPeer(nPeerID);
	pZone->RegisterElement(pBlip);
	TheRadar->ms_RadarTrace[TheRadar->GetActualBlipArrayIndex(nBlipIndex)].bMultiplayerState = !Game.IsLocalPlayer(nPeerID);
	lsc_registerRadarBlip(L, pBlip->GetID());
	return 1;
}

int mp_lsn_AddBlipForCoord(lua_State* L) {
	CVector vPos;
#ifdef GTA_LIBERTY
	int32 nPeerID = lsc_pop_peer_id_from_stack(L, 1, -1);
	if (!lua_istable(L, 1)) return 0;
	lsc_getVectorFromStack(vPos, L, 1, true);
	int32 nColor = (lua_gettop(L) >= 2) ? lua_tonumber(L, 2) : 5;
#else
	int32 nPeerID = lua_tonumber(L, 1);
	lsc_getVectorFromStack(vPos, L, 2, true);
	int32 nColor = (lua_gettop(L) >= 2) ? lua_tonumber(L, 3) : 5;
#endif
	int32 nBlipIndex = TheRadar->SetCoordBlip(eBlipType::BLIP_COORD, vPos, nColor, eBlipDisplay::BLIP_DISPLAY_BOTH);
	if (lua_gettop(L) >= 2)
	{
#ifdef GTA_LIBERTY
		int32 nIconID = lua_tonumber(L, 3);
#else
		int32 nIconID = lua_tonumber(L, 4);
#endif
		TheRadar->SetBlipSprite(nBlipIndex, nIconID);
		TheRadar->ChangeBlipScale(nBlipIndex, 2);
	}

#ifdef GTA_LIBERTY
	if (lua_gettop(L) >= 4 && lua_isboolean(L, 4))
	{
		bool bShortRange = lua_toboolean(L, 4);
		TheRadar->ms_RadarTrace[TheRadar->GetActualBlipArrayIndex(nBlipIndex)].m_bShortRange = bShortRange;
	}
#else
	if (lua_gettop(L) >= 4 && lua_isboolean(L, 5))
	{
		bool bShortRange = lua_toboolean(L, 5);
		TheRadar->ms_RadarTrace[TheRadar->GetActualBlipArrayIndex(nBlipIndex)].bShortRange = bShortRange;
	}
#endif

	cMultiGame& Game = TheMPGame;
	sRadarBlip* pBlip = new sRadarBlip(nBlipIndex);
	cInterestZone* pZone = Game.m_ZoneManager.GetZoneByPeer(nPeerID);
	pZone->RegisterElement(pBlip);
	TheRadar->ms_RadarTrace[TheRadar->GetActualBlipArrayIndex(nBlipIndex)].bMultiplayerState = !Game.IsLocalPlayer(nPeerID);
	lsc_registerRadarBlip(L, pBlip->GetID());
	return 1;
}

int mp_lsn_UpdateBlipCoord(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	int32* pHandleID = (int32*)luaL_checkudata(L, 1, "radarblip");
	if (!pHandleID || *pHandleID == -1) return 0;
	sRadarBlip* pBlip = (sRadarBlip*)Game.GetEntityForHandle(Game.LocalPlayerID(), *pHandleID);
	if (pBlip)
	{
		CVector vPos;
		lsc_getVectorFromStack(vPos, L, 2, true);
		sRadarTrace* pTrace = &TheRadar->ms_RadarTrace[pBlip->m_nBlipIndex];
		pTrace->m_vecPos = vPos;
	}
	return 0;
}

/* leftover */
int mp_lsn_AddLocalBlipForCoord(lua_State* L) {
#ifdef GTA_LIBERTY
	int32 nPeerID = lsc_pop_peer_id_from_stack(L, 1, -1);
#endif
#if !defined(FIX_BUGS) && !defined(GTA_LIBERTY)
	if (!lua_istable(L, 1)) return 0; // leftover
#endif
	CVector vPos;
	lsc_getVectorFromStack(vPos, L, 1, true);
	int32 nColor = (lua_gettop(L) >= 2)  ? lua_tonumber(L, 2) : 5;
	int32 nBlipIndex = TheRadar->SetCoordBlip(eBlipType::BLIP_COORD, vPos, nColor, eBlipDisplay::BLIP_DISPLAY_BOTH);
	if (lua_gettop(L) >= 2) {
		int32 nIconID = lua_tonumber(L, 3);
		TheRadar->SetBlipSprite(nBlipIndex, nIconID);
		TheRadar->ChangeBlipScale(nBlipIndex, 2);
	}
	lua_pushnumber(L, nBlipIndex);
	return 1;
}

int mp_lsn_RemoveLocalRadarBlip(lua_State* L) {
	int32 nIndex = lua_tonumber(L, 1);
	sRadarTrace* pTrace = &TheRadar->ms_RadarTrace[TheRadar->GetActualBlipArrayIndex(nIndex)];
	pTrace->bInUse = false;
	pTrace->m_eBlipType = eBlipType::BLIP_NONE;
	pTrace->eBlipDisplay = eBlipDisplay::BLIP_DISPLAY_NEITHER;
	pTrace->bUnused = false;
	pTrace->m_eRadarSprite = eRadarSprite::RADAR_SPRITE_NONE;
	return 0;
}

int mp_lsn_SetBlipVisibleForPlayerState(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	int32* pHandleID = (int32*)luaL_checkudata(L, 1, "radarblip");
	if (!pHandleID || *pHandleID == -1) return 0;
	sRadarBlip* pBlip = (sRadarBlip*)Game.GetEntityForHandle(Game.LocalPlayerID(), *pHandleID);
	if (!pBlip) return 0;
	uint8 nSlot = lua_tonumber(L, 2);
	bool bEnabled = lua_toboolean(L, 3);
	sRadarTrace* pTrace = &TheRadar->ms_RadarTrace[pBlip->m_nBlipIndex];
	uint8 mask = bEnabled ? (pTrace->m_nPlayerMaskMG | BIT(nSlot)) : (pTrace->m_nPlayerMaskMG & ~BIT(nSlot));
	pTrace->m_nPlayerMaskMG = mask;
	return 0;
}

/* leftover */
int mp_lsn_DrawBlip(lua_State* L) {
	luaL_checknumber(L, 2);
	luaL_checknumber(L, 3);
	return 0;
}

void inline select_color_and_scale(CPickup* pPick, int &color, uint16& scale) {
	scale = 2;
	switch (pPick->m_eModelIndex) {
	case MI_BRASS_KNUCKLES:
	case MI_SCREWDRIVER:
	case MI_GOLFCLUB:
	case MI_NIGHTSTICK:
	case MI_KNIFE:
	case MI_BASEBALL_BAT:
	case MI_HAMMER:
	case MI_MEAT_CLEAVER:
	case MI_MACHETE:
	case MI_KATANA:
	case MI_CHAINSAW:
		color = 0xCFCFFFFF;
		break;
	case MI_GRENADE:
	case MI_TEARGAS:
	case MI_MOLOTOV:
		color = 0x9F9FFFFF;
		break;
	case MI_COLT45:
	case MI_PYTHON:
		color = 0xCFFF00FF;
		break;
	case MI_SHOTGUN:
	case MI_SPAS12_SHOTGUN:
	case MI_STUBBY_SHOTGUN:
		color = 0x7FFF7FFF;
		break;
	case MI_SKOR:
	case MI_UZI:
	case MI_SILENCEDINGRAM:
	case MI_MP5:
		color = 0xFFFF7FFF;
		break;
	case MI_M16:
	case MI_AK47:
		color = 0xFFCF7FFF;
		break;
	case MI_ROCKETLAUNCHER:
	case MI_FLAMETHROWER:
	case MI_M249:
	case MI_MINIGUN:
		color = 0xFF9FFFFF;
		break;
	case MI_SNIPERRIFLE:
	case MI_LASERSCOPE:
		color = 0xFF7FCFFF;
		break;
	default:
		if (pPick->m_eModelIndex == MI_PICKUP_HEALTH) { // health
			color = 0xFFDFDFFF;
		}
		else if (pPick->m_eModelIndex == MI_PICKUP_BODYARMOUR) { // bodyarmour
			color = 0xBFFFBFFF;
		}
		else if (pPick->m_eModelIndex == MI_PICKUP_GOOD_CAR || pPick->m_eModelIndex == MI_PICKUP_BAD_CAR) {
			// goodcar || badcar
			scale = 3;
			color = 0xB5FC00FF;
		}
		else {
			color = 1;
		}
	}
}

int mp_lsn_DrawPickups(lua_State* L) {
	CPlayerPed* pPed = FindPlayerPed();
	static int32 blip_ids[NUMPICKUPS];
	static bool has_init = false;
	if (!has_init) { // gcc?
		memset(blip_ids, 0, sizeof(blip_ids));
		has_init = true;
	}

	for (int32 index = 0; index < NUMPICKUPS; index++) {
		CPickup* pPick = &CPickups::aPickUps[index];
		if (blip_ids[index]) {
			TheRadar->ClearBlip(blip_ids[index]);
			blip_ids[index] = 0;
		}

		if (!pPick->m_bRemoved && pPick->m_eType != ePickupType::PICKUP_NONE)
		{
#ifndef GTA_LIBERTY
			if (pPick->m_eType == ePickupType::PICKUP_NETWORK_1 && pPick->m_eModelIndex == MI_PICKUP_BRIEFCASE)
			{
				int32 id = TheRadar->SetCoordBlip(eBlipType::BLIP_COORD, pPick->m_vecPos, eBlipColour::RADAR_TRACE_YELLOW, eBlipDisplay::BLIP_DISPLAY_BOTH);
				TheRadar->SetBlipSprite(id, RADAR_SPRITE_MPBOMB);
				TheRadar->ChangeBlipScale(id, 2);
				blip_ids[index] = id;
			}
			else
#endif
			if (pPick->m_eType == ePickupType::PICKUP_NETWORK_1 || pPick->m_eType == ePickupType::PICKUP_NETWORK_2)
			{
				int32 id = TheRadar->SetCoordBlip(eBlipType::BLIP_COORD, pPick->m_vecPos, eBlipColour::RADAR_TRACE_YELLOW, eBlipDisplay::BLIP_DISPLAY_BOTH);
				TheRadar->SetBlipSprite(id, RADAR_SPRITE_POWERUP);
				TheRadar->ChangeBlipScale(id, 2);
				blip_ids[index] = id;
			}
			else
			{
				float fDist = Distance(pPed->GetPosition(), pPick->m_vecPos);
				float fMaxDist = luaL_checknumber(L, 1);
				if (fDist < fMaxDist) {
					int nColor = 0;
					uint16 nScale = 0;
					select_color_and_scale(pPick, nColor, nScale);
					int32 id = TheRadar->SetCoordBlip(eBlipType::BLIP_COORD, pPick->m_vecPos, nColor, eBlipDisplay::BLIP_DISPLAY_BOTH);
					TheRadar->ChangeBlipScale(id, nScale);
					blip_ids[index] = id;
				}
			}
		}
	}
	return 1; // not 0?
}

int mp_lsn_SetRadarBlipColour(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	int32* pHandleID = (int32*)luaL_checkudata(L, 1, "radarblip");
	if (!pHandleID || *pHandleID == -1) return 0;
	sRadarBlip* pBlip = (sRadarBlip*)Game.GetEntityForHandle(Game.LocalPlayerID(), *pHandleID);
	if (!pBlip)
		return 0;
	uint32 nColor = lua_tonumber(L, 2);
#ifndef GTA_LIBERTY
	if (nColor >= NUM_RADAR_TRACE_COLOURS) // 17
		nColor = nColor << 8;
#endif
	TheRadar->ms_RadarTrace[pBlip->m_nBlipIndex].m_nColor = nColor;
	return 0;
}

int mp_lsn_ShowRadar(lua_State* L) {
#ifdef GTA_LIBERTY
	if (lua_isboolean(L, 1)) {
		CHud::bScriptDontDisplayRadar = !lua_toboolean(L, 1);
		return 0;
	}
	lua_pushboolean(L, CHud::bScriptDontDisplayRadar);
#else
	if (lua_isboolean(L, 1)) {
		TheHud->m_RadarMode = lua_toboolean(L, 1) ? RADAR_DEFAULT_ON : RADAR_DISABLED;
		return 0;
	}
	lua_pushboolean(L, TheHud->m_RadarMode != RADAR_DISABLED);
#endif
	return 1;
}

#ifndef GTA_LIBERTY
int mp_lsn_SetRadarToTeamColour(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	int32* pHandleID = (int32*)luaL_checkudata(L, 1, "radarblip");
	if (!pHandleID || *pHandleID == -1) return 0;
	sRadarBlip* pBlip = (sRadarBlip*)Game.GetEntityForHandle(Game.LocalPlayerID(), *pHandleID);
	if (!pBlip) return 0;
	uint8 nTeamId = lua_tonumber(L, 2);
	CRGBA col = *Game.GetColor(nTeamId);
	TheRadar->ms_RadarTrace[pBlip->m_nBlipIndex].m_nColor = CRGBA_PACK(col.r, col.g, col.b, col.a);
	return 0;
}
#endif

static const luaL_reg ls_radar_lib[] = {
	{"AddSpriteBlipForContactPoint", mp_lsn_AddSpriteBlipForContactPoint},
	{"AddBlipForCoord",              mp_lsn_AddBlipForCoord},
	{"UpdateBlipCoord",              mp_lsn_UpdateBlipCoord},
	{"RemoveBlip",                   mp_lsn_RemoveRadarBlip},
	{"AddLocalBlipForCoord",         mp_lsn_AddLocalBlipForCoord},
	{"RemoveLocalRadarBlip",         mp_lsn_RemoveLocalRadarBlip},
	{"SetBlipVisibleForPlayerState", mp_lsn_SetBlipVisibleForPlayerState},
	{"DrawBlip",                     mp_lsn_DrawBlip},
	{"DrawPickups",                  mp_lsn_DrawPickups},
	{"SetRadarBlipColour",           mp_lsn_SetRadarBlipColour},
#ifndef GTA_LIBERTY
	{"SetRadarToTeamColour",         mp_lsn_SetRadarToTeamColour},
#endif
	{"ShowRadar",                    mp_lsn_ShowRadar},
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_radar_lib, (12 + 1), (11 + 1));


void lscript_open_radar() {
	cLWrapper& wrapper = cLWrapper::Instance();
	wrapper.CreateLibrary(ls_radar_blip, "radarblip");
	wrapper.CreateGlobalLibrary(ls_radar_lib, nil);
}