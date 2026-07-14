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

int mp_lsn_EntityIsDead(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	bool bIsDead = false;

	sPed* pPed = nil;
	if (lsc_isPlayerUserData(L, 1)) {
		int32 nPlayerID = lsc_getPlayer(L, 1);
		pPed = (sPed*)Game.GetEntityForHandle(nPlayerID, eElementID::MG_ELEMENT_PLAYER_PED_ID);
	}
	else {
		pPed = (sPed*)lsc_get_entity(L, 1);
	}

	if (pPed == nil) {
		bIsDead = true;
	}
	else if (pPed->HasCapability(sVehicle::Capability())) {
		bIsDead = (pPed->GetSync().vehicle->m_nStatus == eEntityStatus::STATUS_WRECKED);
	}
	else if (pPed->HasCapability(sPlayer::Capability())) {
		sElement* pPlayer = Game.GetEntityForHandle(pPed->GetOwner(), eElementID::MG_ELEMENT_PLAYER_PED_ID);
		if (pPlayer)
			bIsDead = (pPlayer->GetSync().ped->m_nHealth < 2);
	}

	lua_pushboolean(L, bIsDead);
	return 1;
}

int mp_lsn_EntityPosition(lua_State* L) {
	CVector pos(0.0f, 0.0f, 0.0f);
	sElement* pElem = lsc_get_entity(L, 1);
	if (pElem == nil) return 0;
	if (pElem->HasCapability(sPlayer::Capability()))
		pos = ((sPlayer*)pElem)->GetPosition();
	else
		pos = ((sElementPhysical*)pElem)->GetSync().ped->GetMatrix().GetPosition();
	lsc_pushVuVector(L, pos);
	return 1;
}


static const luaL_reg entity_lib[] = {
	{"IsDead",   mp_lsn_EntityIsDead},
	{"Position", mp_lsn_EntityPosition},
	{NULL, NULL}
};
VALIDATE_LUA_LIB(entity_lib, (2 + 1), (2 + 1));

void lscript_open_entity() {
	cLWrapper& wrapper = cLWrapper::Instance();
	wrapper.CreateGlobalLibrary(entity_lib, nil);
}