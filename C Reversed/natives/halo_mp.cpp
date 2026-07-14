/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/natives/public.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/MultiGame.h"

uint16* ls_get_halo(lua_State* L, int ud) {
	return (uint16*)luaL_checkudata(L, ud, "halo");
}

int mp_lsn_RemoveHalo(lua_State* L) {
	cMultiGame& pGame = TheMPGame;
	uint16* pHandle = ls_get_halo(L, 1);
	if (!pHandle || *pHandle == -1) return 0;
	pGame.m_haloManager.Remove(*pHandle);
	*pHandle = -1;
	return 0;
}

static const luaL_reg ls_halo_obj[] = {
	{"Remove", mp_lsn_RemoveHalo},
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_halo_obj, (1 + 1), (1 + 1));


void register_halo_element(lua_State* L, uint16 id) {
	uint16* pHandle = (uint16*)lua_newuserdata(L, sizeof(uint16));
	*pHandle = id;
	luaL_getmetatable(L, "halo");
	lua_setmetatable(L, -2);
}

int mp_lsn_SetHalo(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	CVector vPos;
	CVector vExtent(7.0f, 7.0f, 7.0f);
	uint32 nColor = 0xFF0000; // brga
	lsc_getVectorFromStack(vPos, L, 1, true);
	int32 nTop = lua_gettop(L);
	if (nTop >= 2) {
		lsc_getVectorFromStackNoEntity(vExtent, L, 2);
	}
	if (nTop >= 3) {
		int32 nColorIndex = lua_tonumber(L, 3);
		CRGBA Color = *Game.GetBlipColor(nColorIndex);
		nColor = BGRA_PACK(Color.r, Color.g, Color.b, 255);
	}
	float fMarkerSize = (nTop >= 4) ? lua_tonumber(L, 4) : (vExtent.x * 0.875f);
	uint16 nHaloID = Game.m_haloManager.Add(vPos, vExtent, nColor, fMarkerSize);
	register_halo_element(L, nHaloID);
	return 1;
}

static const luaL_reg ls_halo_lib[] = {
	{"SetHalo",    mp_lsn_SetHalo},
	{"RemoveHalo", mp_lsn_RemoveHalo},
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_halo_lib, (2 + 1), (2 + 1));

void lscript_open_halo() {
	cMultiGame& pGame = TheMPGame;
	cLWrapper& wrapper = cLWrapper::Instance();
	wrapper.CreateLibrary(ls_halo_obj, "halo");
	wrapper.CreateGlobalLibrary(ls_halo_lib, nil);
}