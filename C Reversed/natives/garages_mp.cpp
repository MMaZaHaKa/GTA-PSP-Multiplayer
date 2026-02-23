/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "Camera.h"
#include "Garages.h"

#include "multiplayer/natives/public.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/MultiGame.h"


static const luaL_reg ls_garage_lib[] = {
	{NULL, NULL},
};
VALIDATE_LUA_LIB(ls_garage_lib, (0 + 1), (0 + 1));

int mp_lsn_CreateGarage(lua_State* L) {
	CVector pos;
	lsc_getVectorFromStack(pos, L, 1, true);
	float fAngle = lua_tonumber(L, 2);
	int32 nHandle = CGarages::AddCrateGarage(pos, fAngle);
	debug("Created New Garage %d\n", nHandle);
	lua_pushnumber(L, nHandle);
	return 1;
}

int mp_lsn_SetVehicleForGarage(lua_State* L) {
	int32 nPlayerID = lua_tonumber(L, 1);
	int32 nHandle = lua_tonumber(L, 2);
	sElementPhysical* pEntity = (sElementPhysical*)lsc_get_entity(L, 3);
	if (pEntity && nHandle >= 0) CGarages::SetVehicleForSSGarage(nPlayerID, nHandle, pEntity);
	return 0;
}

int mp_lsn_SetPlayersVehicleForGarage(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int32 nHandle = lua_tonumber(L, 1);
	int32 nPlayerID = Game.LocalPlayerID();
	if (lsc_isPlayerUserData(L, 2)) nPlayerID = lsc_getPlayer(L, 2);
	sPed* pPed = (sPed*)Game.GetEntityForHandle(nPlayerID, eElementID::MG_ELEMENT_PLAYER_PED_ID); // cMultiGame::GetPlayerPed
	sElementPhysical* pEntry = nil;
	if (pPed) {
		// lmao code block
		int16 nVehicleID = pPed->GetSync().ped->GetVehicleID();
		sElementPhysical* pVehicle = nil;
#ifdef THIS_IS_STUPID // probably macro shit
		if(nVehicleID != -1)
			pVehicle = (sElementPhysical*)Game.GetEntityForHandle(pPed->GetOwner(), nVehicleID);

		if (pVehicle) {
			int16 nVehicleID2 = pPed->GetSync().ped->GetVehicleID(); // kek here
			if (nVehicleID2 != -1)
				pEntry = (sElementPhysical*)Game.GetEntityForHandle(pPed->GetOwner(), nVehicleID2);
		}
#else
		if (nVehicleID != -1)
			pEntry = (sElementPhysical*)Game.GetEntityForHandle(pPed->GetOwner(), nVehicleID);
#endif
	}
	if (pEntry && nHandle >= 0) CGarages::SetVehicleForSSGarage(nPlayerID, nHandle, pEntry);
	return 0;
}

int mp_lsn_HasGarageAcceptedVehicle(lua_State* L) {
	int32 nHandle = lua_tonumber(L, 1);
	bool bAccepted = CGarages::HasSSGarageAcceptedVehicle(nHandle);
	lua_pushboolean(L, bAccepted);
	return 1;
}

int mp_lsn_IsVehicleInGarage(lua_State* L) {
	int32 nHandle = lua_tonumber(L, 1);
	sElementPhysical* pElement = (sElementPhysical*)lsc_get_entity(L, 2);
	if (pElement == nil) return 0;
	CEntity* pEntity = pElement->GetOwner() != cMultiGame::Instance().LocalPlayerID() ? pElement->GetPhysical() : pElement->GetEntity();
	bool bIsInside = false;
	if (pEntity) bIsInside = CGarages::aGarages[nHandle].IsEntityEntirelyInside3D(pEntity, 0.0f);
	lua_pushboolean(L, bIsInside);
	return 1;
}

int mp_lsn_DoesGarageContainSSVehicle(lua_State* L) {
	int32 nHandle = lua_tonumber(L, 1);
	CVehicle* pTargetCar = CGarages::aGarages[nHandle].m_pSSTargetCar;
	lua_pushboolean(L, pTargetCar != nil);
	return 1;
}

int mp_lsn_SetGarageState(lua_State* L) {
	int32 nHandle = lua_tonumber(L, 1);
	uint8 eState = lua_tonumber(L, 2);
	CGarages::aGarages[nHandle].m_eGarageState = eState;
	return 0;
}

int mp_lsn_GetGarageState(lua_State* L) {
	int32 nHandle = lua_tonumber(L, 1);
	uint8 eState = CGarages::aGarages[nHandle].m_eGarageState;
	lua_pushnumber(L, eState);
	return 1;
}

int mp_lsn_IsSSGarageStateChanging(lua_State* L) {
	if (!lua_isnumber(L, 1)) return 0;
	int32 nHandle = lua_tonumber(L, 1);
	bool bIsChanging = CGarages::aGarages[nHandle].m_bSSGarageStateChanging;
	lua_pushboolean(L, bIsChanging);
	return 0;
}

static const luaL_reg ls_garages_lib[] = {
	{"CreateGarage",               mp_lsn_CreateGarage},
	{"SetVehicleForGarage",        mp_lsn_SetVehicleForGarage},
	{"SetPlayersVehicleForGarage", mp_lsn_SetPlayersVehicleForGarage},
	{"HasGarageAcceptedVehicle",   mp_lsn_HasGarageAcceptedVehicle},
	{"IsVehicleInGarage",          mp_lsn_IsVehicleInGarage},
	{"DoesGarageContainSSVehicle", mp_lsn_DoesGarageContainSSVehicle},
	{"SetGarageState",             mp_lsn_SetGarageState},
	{"GetGarageState",             mp_lsn_GetGarageState},
	{"IsSSGarageStateChanging",    mp_lsn_IsSSGarageStateChanging},
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_garages_lib, (9 + 1), (9 + 1));


void lscript_open_garages() {
	cMultiGame& pGame = TheMPGame;
	cLWrapper& wrapper = cLWrapper::Instance();
	wrapper.CreateLibrary(ls_garage_lib, "garage");
	wrapper.CreateGlobalLibrary(ls_garages_lib, nil);
}