/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "PlayerPed.h"
#include "World.h"

#include "multiplayer/natives/public.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/elements/sVehicle.h"
#include "multiplayer/MultiGame.h"

int mp_lsn_PedIsDead(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	bool bIsDead = false;

	sPed* pPed = nil;
	if (lsc_isPlayerUserData(L, 1)) {
		int nPlayerID = lsc_getPlayer(L, 1);
		pPed = (sPed*)Game.GetEntityForHandle(nPlayerID, eElementID::MG_ELEMENT_PLAYER_PED_ID);
	}
	else {
		pPed = (sPed*)lsc_get_entity(L, 1);
	}

	if (pPed == nil) {
		bIsDead = true;
	}
	else if (pPed->HasCapability(sVehicle::Capability()))
	{
		sVehicleSync* pSync = pPed->GetSync().vehicle;
		TODO();
		//bIsDead = pSync->field_C2 == 5;
	}
	else if (pPed->HasCapability(sPlayer::Capability()))
	{
		sElement* pPlayer = Game.GetEntityForHandle(pPed->GetOwner(), eElementID::MG_ELEMENT_PLAYER_PED_ID);
		if (pPlayer)
			bIsDead = (pPlayer->GetSync().ped->m_nHealth < 2);
	}

	lua_pushboolean(L, bIsDead);
	return 1;
}

int mp_lsn_PedPosition(lua_State* L) {
	CVector pos(0.0f, 0.0f, 0.0f);
	sElement* pElem = lsc_get_entity(L, 1);
	if (pElem == nil) return 0;
	if (pElem->HasCapability(sPlayer::Capability()))
		pos = ((sPlayer*)pElem)->GetPosition();
	else
		pos = ((sPed*)pElem)->GetSync().ped->GetMatrix().GetPosition();
	lsc_pushVuVector(L, pos);
	return 1;
}


static const luaL_reg ped_lib[] = {
	{"IsDead",   mp_lsn_PedIsDead},
	{"Position", mp_lsn_PedPosition},
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ped_lib, (2 + 1), (2 + 1));

void lscript_open_ped() {
	cLWrapper& wrapper = cLWrapper::Instance();
	wrapper.CreateGlobalLibrary(ped_lib, nil);
}