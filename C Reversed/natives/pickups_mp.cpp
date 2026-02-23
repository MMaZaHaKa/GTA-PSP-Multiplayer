/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "Pickups.h"
#include "ModelIndices.h"
#include "World.h"


#include "multiplayer/natives/public.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sPickup.h"


int mp_lsn_RemovePickup(lua_State* L) {
	int32* pData = (int32*)luaL_checkudata(L, 1, "pickup");
	if (pData == nil)
		return 0;
	int32 nHandle = (int32)*pData;
	if (nHandle != -1) {
		CPickups::RemovePickUp(nHandle);
		*pData = -1;
	}
	return 0;
}

int mp_lsn_IsCollectable(lua_State* L) {
	int32* pData = (int32*)luaL_checkudata(L, 1, "pickup");
	if (pData == nil)
		return 0;
	int32 nHandle = (int32)*pData;
	if (nHandle == -1)
		return 0;
	int32 nIndex = CPickups::GetActualPickupIndex(nHandle);
	lua_pushboolean(L, (!CPickups::aPickUps[nIndex].m_bRemoved));
	return 1;
}

int mp_lsn_Position(lua_State* L) {
	int32* pData = (int32*)luaL_checkudata(L, 1, "pickup");
	if (pData == NULL) return 0;
	int32 nHandle = (int32)*pData;
	if (nHandle == -1) return 0;
	int32 nIndex = CPickups::GetActualPickupIndex(nHandle);
	CPickup* pEntry = &CPickups::aPickUps[nIndex];
	lsc_pushVuVector(L, pEntry->m_vecPos);
	return 1;
}

static const luaL_reg ls_pickup_lib[] = {
	{"Remove",        mp_lsn_RemovePickup},
	{"IsCollectable", mp_lsn_IsCollectable},
	{"Position",      mp_lsn_Position},
	{NULL, NULL},
};
VALIDATE_LUA_LIB(ls_pickup_lib, (3 + 1), (3 + 1));

void lsc_registerPickup(lua_State* L, int32 nHandle) {
	int32* data = (int32*)lua_newuserdata(L, sizeof(int32));
	*data = nHandle;
	luaL_getmetatable(L, "pickup");
	lua_setmetatable(L, -2);
}

int mp_lsn_CreatePickup(lua_State* L) {
	CVector pos;
	cMultiGame& Game = cMultiGame::Instance();
	int nModelID = lua_tonumber(L, 1);
	lsc_getVectorFromStack(pos, L, 2, true);
	if (pos.z <= MAP_Z_LOW_LIMIT)
		return luaL_argerror(L, 2, "vector must have a Z coordinate for pickups!");
	int nQuantity = lua_tonumber(L, 3);
	bool bIsPowerUp = isPickupPowerup(nModelID);
	if (!Game.bPowerUpOn && bIsPowerUp)
#ifdef FIX_BUGS
		return 0;
#else
		return 1;
#endif
#ifndef GTA_LIBERTY
	bIsPowerUp = bIsPowerUp || (nModelID == MI_PICKUP_BRIEFCASE);
#endif
	uint8 eType = bIsPowerUp ? ePickupType::PICKUP_NETWORK_1 : ePickupType::PICKUP_ON_STREET;
	int32 nHandle = CPickups::GenerateNewOne(pos, nModelID, eType, nQuantity, 0, false, nil);
	lsc_registerPickup(L, nHandle);
	sPickup* pElem = new sPickup(nHandle);
	cInterestZone* pZone = Game.m_ZoneManager.GetZoneByPeer(MP_HOST_INDEX);
	pZone->RegisterElement(pElem);
	return 1; // 1? not push
}

/* TODO(MP)#2 */
int mp_lsn_DoesPowerupExist(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int32 nCount = 0;
	for (int32 nPlayerID = 0; nPlayerID < 10; nPlayerID++) {
		sPlayer* pPlayer = cMultiGame::Instance().GetPlayer(nPlayerID);
		// TODO(MP): missing code
	}
	if (nCount == 0) {
		for (int nIndex = 0; nIndex < NUMPICKUPS; nIndex++) {
			CPickup* pEntry = &CPickups::aPickUps[nIndex];
			if (pEntry->m_eType == PICKUP_NETWORK_1 || pEntry->m_eType == PICKUP_NETWORK_2)
				nCount++;
		}
	}
	lua_pushnumber(L, nCount);
	return 1;
}

int mp_lsn_RegenerateAllPickups(lua_State* L) {
	CPickups::PassTime(120 * 1000);
	return 0;
}

#ifndef GTA_LIBERTY
int mp_lsn_GetFlagballPosition(lua_State* L) {
	CVector pos = TheMPGame.GetFlagBallPosition();
	lsc_pushVuVector(L, pos);
	return 1;
}

int mp_lsn_DestroyFlagBall(lua_State* L) {
	CPickups::DestroyFlagBall();
	return 0;
}
#endif

static const luaL_reg ls_pickups_lib[] = {
	{"CreatePickup",         mp_lsn_CreatePickup},
	{"DoesPowerupExist",     mp_lsn_DoesPowerupExist},
	{"RegenerateAllPickups", mp_lsn_RegenerateAllPickups},
#ifndef GTA_LIBERTY
	{"GetFlagballPosition",  mp_lsn_GetFlagballPosition},
	{"DestroyFlagBall",      mp_lsn_DestroyFlagBall},
#endif
	{NULL, NULL},
};
VALIDATE_LUA_LIB(ls_pickups_lib, (5 + 1), (3 + 1));


void lscript_open_pickups() {
	cLWrapper& wrapper = cLWrapper::Instance();
	wrapper.CreateLibrary(ls_pickup_lib, "pickup");
	wrapper.CreateGlobalLibrary(ls_pickups_lib, nullptr);
}