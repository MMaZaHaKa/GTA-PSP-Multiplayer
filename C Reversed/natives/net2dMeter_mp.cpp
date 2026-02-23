/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/natives/public.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sTextSprite.h"
#include "multiplayer/elements/sNetMeter2d.h"


#ifndef GTA_LIBERTY
sNetMeter2d* lsc_get_net_meter_2d(lua_State* L) {
	cMultiGame& pGame = cMultiGame::Instance();
	void* pData = luaL_checkudata(L, 1, "net2dMeter");
	if (!pData) return nil;
	int nHandle = *((int*)pData);
	if (nHandle == -1) return nil;
	return (sNetMeter2d*)pGame.GetEntityForHandle(pGame.LocalPlayerID(), nHandle);
}

int mp_lsn_FillRatio(lua_State* L) {
	sNetMeter2d* pMeter = lsc_get_net_meter_2d(L);
	if (!pMeter) return 0;
	if (lua_isnumber(L, 2)) {
		float fillRatio = lua_tonumber(L, 2);
		fillRatio = Clamp(fillRatio, 0.0f, 1.0f);
		pMeter->SetFillRatio(fillRatio);
		return 0;
	}

	lua_pushnumber(L, pMeter->GetFillRatio());
	return 1;
}

int mp_lsn_Alpha(lua_State* L) {
	sNetMeter2d* pMeter = lsc_get_net_meter_2d(L);
	if (!pMeter) return 0;
	if (lua_isnumber(L, 2)) {
		uint8 alpha = lua_tonumber(L, 2);
		pMeter->SetAlpha(alpha);
		return 0;
	}

	lua_pushnumber(L, pMeter->GetAlpha());
	return 1;
}

int mp_lsn_Flashing(lua_State* L) {
	sNetMeter2d* pMeter = lsc_get_net_meter_2d(L);
	if (!pMeter) return 0;
	if (lua_isboolean(L, 2)) {
		bool flashing = lua_toboolean(L, 2);
		pMeter->SetFlashing(flashing);
		return 0;
	}

	lua_pushnumber(L, pMeter->GetFlashing());
	return 1;
}

int mp_lsn_nColour(lua_State* L) {
	sNetMeter2d* pMeter = lsc_get_net_meter_2d(L);
	if (!pMeter) return 0;
	if (lua_isnumber(L, 2)) {
		CRGBA color = CRGBA_UNPACK_LEGACY(lsc_getColor(L, 2));
		pMeter->SetColour(color);
		return 0;
	}

	CRGBA* pColour = pMeter->GetColour();
	lua_pushnumber(L, RGB24_PACK(pColour->r, pColour->g, pColour->b));
	return 1;
}

int mp_lsn_UseTitle(lua_State* L) {
	sNetMeter2d* pMeter = lsc_get_net_meter_2d(L);
	if (!pMeter || !lua_isnumber(L, 2)) // lua_isboolean?
		return 0;

	bool bUseTitle = lua_tonumber(L, 2);
	pMeter->SetUseTitle(bUseTitle); // (bool)
	return 0;
}

int mp_lsn_SetTitleKey(lua_State* L) {
	sNetMeter2d* pMeter = lsc_get_net_meter_2d(L);
	if (!pMeter || !lua_isstring(L, 2))
		return 0;

	const char* sTitleKey = lua_tostring(L, 2);
	pMeter->SetTitleKey(sTitleKey);
	return 0;
}

void push_NetMeter2d(lua_State* L, sNetMeter2d* pNetMeter) {
	int* pHandleID = (int*)lua_newuserdata(L, sizeof(int));
	*pHandleID = pNetMeter->GetID();
	luaL_getmetatable(L, "net2dMeter");
	lua_setmetatable(L, -2);
}

int mp_lsn_RemoveNetMeter2d(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int* pHandleID = (int*)luaL_checkudata(L, 1, "net2dMeter");
	if (!pHandleID || *pHandleID == -1) return 0;
	sNetMeter2d* pMeter = (sNetMeter2d*)Game.GetEntityForHandle(Game.LocalPlayerID(), *pHandleID);
	if (pMeter) delete pMeter;
	*pHandleID = -1;
	return 0;
}

static const luaL_reg ls_net2dMeter_obj[] = {
	{"FillRatio",   mp_lsn_FillRatio},
	{"Alpha",       mp_lsn_Alpha},
	{"Flashing",    mp_lsn_Flashing},
	{"Colour",      mp_lsn_nColour},
	{"UseTitle",    mp_lsn_UseTitle},
	{"SetTitleKey", mp_lsn_SetTitleKey},
#ifdef FIX_BUGS
	{"Remove",      mp_lsn_RemoveNetMeter2d},
	{"__gc",        mp_lsn_RemoveNetMeter2d},
#endif
	{NULL, NULL}
};
#ifdef FIX_BUGS
VALIDATE_LUA_LIB(ls_net2dMeter_obj, (6 + 1) + 2, (0 + 1));
#else
VALIDATE_LUA_LIB(ls_net2dMeter_obj, (6 + 1), (0 + 1));
#endif



int mp_lsn_NetMeter2d(lua_State* L) {
	int32 nPeerID = lua_tonumber(L, 1);
	float fPosX = lua_tonumber(L, 2);
	float fPosY = lua_tonumber(L, 3);
	float fBarWidth = lua_tonumber(L, 4);
	float fBarHeight = lua_tonumber(L, 5);
	sNetMeter2d* pMeter = new sNetMeter2d(nPeerID, fPosX, fPosY, fBarWidth, fBarHeight);
	push_NetMeter2d(L, pMeter);
	return 1;
}

static const luaL_reg ls_net2dMeter_lib[] = {
	{"NetMeter2d", mp_lsn_NetMeter2d},
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_net2dMeter_lib, (1 + 1), (0 + 1));


void lscript_open_net2dMeter() {
	cLWrapper& wrapper = cLWrapper::Instance();
	wrapper.CreateLibrary(ls_net2dMeter_obj, "net2dMeter");
	wrapper.CreateGlobalLibrary(ls_net2dMeter_lib, nil);
}
#endif