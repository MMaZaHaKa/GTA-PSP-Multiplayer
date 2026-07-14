/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "World.h"
#include "Vehicle.h"
#include "Wanted.h"
#include "Automobile.h"
#include "Streaming.h"
#include "Hud.h"
#include "Text.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"

#include "multiplayer/natives/public.h"


int mp_lsn_cheatweapons(lua_State* L) {
	TheHud->SetHelpMessage(TheText.Get("CHEAT2"), true);
	CPlayerPed* pPed = FindPlayerPed();
#ifdef FIX_BUGS // fix from WeaponCheat1()
	CStreaming::RequestModel(MI_BASEBALL_BAT, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_COLT45, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_UZI, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_SHOTGUN, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_SNIPERRIFLE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_ROCKETLAUNCHER, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_MOLOTOV, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_GRENADE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_FLAMETHROWER, STREAMFLAGS_DONT_REMOVE);
	CStreaming::LoadAllRequestedModels(false);

	FindPlayerPed()->GiveWeapon(WEAPONTYPE_BASEBALLBAT, 1);
#else
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_BASEBALLBAT, 0); // bug?
#endif
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_COLT45, 100);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_UZI, 100);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_SHOTGUN, 20);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_SNIPERRIFLE, 5);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_ROCKET, 5);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_MOLOTOV, 5);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_GRENADE, 5);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_FLAMETHROWER, 200);

#ifdef FIX_BUGS
	CStreaming::SetModelIsDeletable(MI_BASEBALL_BAT);
	CStreaming::SetModelIsDeletable(MI_COLT45);
	CStreaming::SetModelIsDeletable(MI_UZI);
	CStreaming::SetModelIsDeletable(MI_SHOTGUN);
	CStreaming::SetModelIsDeletable(MI_SNIPERRIFLE);
	CStreaming::SetModelIsDeletable(MI_ROCKETLAUNCHER);
	CStreaming::SetModelIsDeletable(MI_MOLOTOV);
	CStreaming::SetModelIsDeletable(MI_GRENADE);
	CStreaming::SetModelIsDeletable(MI_FLAMETHROWER);
#endif
	return 0;
}

int mp_lsn_cheatmoney(lua_State* L) {
	CWorld::Players[CWorld::PlayerInFocus].m_nMoney += 250000;
	TheHud->SetHelpMessage(TheText.Get("CHEAT6"), true);
	return 0;
}

int mp_lsn_cheatarmor(lua_State* L) {
	TheHud->SetHelpMessage(TheText.Get("CHEAT4"), true);
	FindPlayerPed()->m_fArmour = 100.0f;
	return 0;
}

int mp_lsn_cheathealth(lua_State* L) {
	TheHud->SetHelpMessage(TheText.Get("CHEAT3"), true);
	FindPlayerPed()->m_fHealth = 100.0f;
	CVehicle* pVeh = FindPlayerVehicle();
	if (pVeh) {
#if !defined(GTA_LIBERTY) && defined(FIX_BUGS)
		pVeh->m_fHealth = pVeh->m_fMaxHealth;
#else
		pVeh->m_fHealth = 1000.0f;
#endif
		if (pVeh->IsCar())
			((CAutomobile*)pVeh)->Damage.SetEngineStatus(0);
	}
	return 0;
}

int mp_lsn_cheatwanted(lua_State* L) {
	TheHud->SetHelpMessage(TheText.Get("CHEAT5"), true);
	CPlayerPed* pPed = FindPlayerPed();
	pPed->m_Wanted.CheatWantedLevel(Min(pPed->m_Wanted.GetWantedLevel() + 2, 6));
	return 0;
}

int mp_lsn_cheatvehicle(lua_State* L) {
	int nModel = lua_gettop(L) ? luaL_checknumber(L, 1) : MI_RHINO;
	CStreaming::FlushRequestList();
	CStreaming::RequestModel(nModel, 0);
	CStreaming::LoadAllRequestedModels(false);
	if (CStreaming::ms_aInfoForModel[nModel].m_loadState == STREAMSTATE_LOADED) {
		int32 nNode = ThePaths.FindNodeClosestToCoors(FindPlayerCoors(), PATH_CAR, 100.0f);
		if (nNode < 0) return 0;
		CAutomobile* pVeh = new CAutomobile(nModel, MISSION_VEHICLE);
		if (pVeh != NULL) {
			CVector tPos = ThePaths.m_pathNodes[nNode].GetPosition();
			tPos.z += 4.0f;
			pVeh->SetPosition(tPos);
			pVeh->SetOrientation(0.0f, 0.0f, DEGTORAD(200.0f));
			pVeh->SetStatus(STATUS_ABANDONED);
			pVeh->m_nDoorLock = CARLOCK_UNLOCKED;
			CWorld::Add(pVeh);
		}
	}
	return 0;
}

void lscript_open_cheats() {
	cLWrapper& wrapper = cLWrapper::Instance();
	lua_register(wrapper.m_luaVM, "cheatweapons", mp_lsn_cheatweapons);
	lua_register(wrapper.m_luaVM, "cheatmoney", mp_lsn_cheatmoney);
	lua_register(wrapper.m_luaVM, "cheatarmor", mp_lsn_cheatarmor);
	lua_register(wrapper.m_luaVM, "cheathealth", mp_lsn_cheathealth);
	lua_register(wrapper.m_luaVM, "cheatwanted", mp_lsn_cheatwanted);
	lua_register(wrapper.m_luaVM, "cheatvehicle", mp_lsn_cheatvehicle);
}