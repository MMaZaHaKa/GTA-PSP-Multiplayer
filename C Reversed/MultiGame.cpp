/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"

#include "Bridge.h"
#include "Script.h"
#include "Object.h"
#include "World.h"
#include "Pools.h"
#include "PathFind.h"
#include "Text.h"
#include "Timer.h"
#include "Streaming.h"
#include "ColStore.h"
#include "Timecycle.h"
#include "Camera.h"
#include "Messages.h"
#include "Garages.h"
#include "Train.h"
#include "Radar.h"
#include "AudioManager.h"
#include "CarCtrl.h"
#include "Script.h"
#include "ModelInfo.h"
#include "ModelIndices.h"
#include "Stats.h"
#include "Pad.h"
#include "DMAudio.h"
#include "main.h"
#include "Clock.h"
#include "Darkel.h"
#include "Frontend.h"
#include "Weather.h"
#include "Population.h"
#include "PlayerPed.h"
#include "WaterLevel.h"
#include "timebars.h"
#include "Hud.h"
#include "FileMgr.h"
#include "multiplayer/elements/sSyncStream.h"
#ifdef GTA_PSP
#include <kernel.h>
#include <psptypes.h>
#include <wlan.h>
#include <pspnet.h>
#include <pspnet_error.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>
#include <pspnet_adhoc_discover.h>
#else
#include "multiplayer/net/emu/NetAdhocCommon.h"
#include "multiplayer/net/emu/Resolve.h"
#include "multiplayer/net/emu/sceNet.h"
#endif
#ifndef GTA_LIBERTY
#include "FerrisWheel.h"
#endif
#ifdef DEBUGMENU
#include "debugmenu.h"
#endif

#include "multiplayer/public.h"
#include "multiplayer/natives/public.h"
#include "multiplayer/events/public.h"
#include "multiplayer/elements/sTextSprite.h"
#include "multiplayer/elements/sPed.h"
#include "multiplayer/elements/sVehicle.h"
#include "multiplayer/MultiGame.h"
#include "multiplayer/GameInstance.h"
#include "multiplayer/LScript.h"
#include "multiplayer/Lobby.h"
#ifndef GTA_PSP
#include "multiplayer/net/emu/Utils.h" // CreateRandMAC
#endif


//GameCoreTick (psp/ps2 coreloop)
//{
//	if ( gIsMultiplayerGame )
//	  mp_game_update_recv(); ->
//			cMultiGame::UpdateReceive();
//					nTime =  m_nUpdateSendTime + m_fTimeStep
//					-> cNetSession::UpdateReceive(nTime)
//										NS->m_nCurTime = nTime; // <<-------------------------------------    update current frame to basic net session timer!!!!!!!!
//										cNetSession::UpdateReceivePvt(this);
//														while 1 cListenInfo::RecvPDPPacket() // << ---------------- read udp data!!!!!!!!!!
//														cNetSession::HandlePacketRecv()
//																		cNetSession::DispatchMessages() // -> output to recv cb
//																						main FireMessageHandler(main packet cb)  m_pPacketDispatcher -> PerformDispatchPacket
//																																															cMultiGame::FireMessageHandler() -> PerformDispatchPacket by packet id
//																																													!!!!!!!!!!!!!!!!!!!!!!!!!!!!					      	cMultiGame::OnGameStateChange
//																																																														cInterestZone::ReceiveGameState // read syncs
//																																																																			sElement::GetSyncWithTime2 
//																																																																					friendElemSync = sElement::CreateSyncFromOther(pBuffWithSync)		
//														cNetSession::DispatchMessages() -> output to recv cb
//										cNetSession::UpdatePendingSent(this);
//														cNetSession::AttemptSendPacket()
//																		cListenInfo::SendPDPPacket() // send ack -------------
//													 
//																																	
//	CGame::Process();
//			CWorld::Process();
//						movingEnt->UpdateAnim();
//									sPed::UpdateAnim() -> apply from last sync animation to cPedMG physical clump //------------- do anim for net ped from sync
//									RpAnimBlendClumpUpdateAnimations(delta 0.0f)
//									
//	
//	if ( gIsMultiplayerGame )
//	  mp_game_update_send();
//			-> cMultiGame::UpdateSend() -> (BIG UPD MG) simsch, etc
//								sPed::Update() -> 
//												update elements // AttachSync from native CPed // Copy animations from native CPed clump to new sync //------------ grab anims from our hidden native clump CPed into new sync
//								cMultiGame::UpdateZonePeersTimeouts();      // UpdateZonePeers in LCS disconn stuff
//								cMultiGame::UpdateZonePeersSync(); // delete dispose stuff sync // <<--------------------------------
//								lsc_update_simsch();                      // ----------------------- do lua!!!
//								cInterestZoneManager::UpdatePeer()
//								cInterestZoneManager::UpdatePlayer() // <<< ---------------------
//																 cInterestZone::SendGameState() // <<----------------- send all our sync snapshots!!!!!!
//																					zone->m_nBasis = NS->m_nCurTime; // <<-------------------------------------    update current frame interest zone!!!!!!!!!
//																					split zone m_nCurTime += startSize ?? todo mean
//								cNetSession::UpdateSend()
//													cListenInfo::SendPDPPacket() // << ------------------------ send main stuff
//													cNetSession::PacketAppend()
//													cNetSession::AttemptSendPacket()
//													
//						
//						
//	RenderScene()
//			MattRenderScene_1todo_world()
//								CMattRenderer::Render()
//											CRenderer::PreRender()
//														cPedMG::PreRender(); //-------- render our net ped
//	RenderEffects()
//			 CRenderer::RenderVehicles() // +peds kek
//								CRenderer::RenderOneNonRoad()
//												cPedMG::Render();		
//}





/*
TODO
#1: implement function (it is just a stub)
#2: function partially implemented (have to re-check it)
 + missing some instructions and/or function calls
#3: code adapted from PSP
 + PSP controls/framework usually are not available in PC
*/

SETTWEAKPATH("Multiplayer");
bool gbMP_DrawPauseScreen = false;
bool gbMP_DrawPauseScreenNoBox = false;
bool gbMP_RenderHudExtras = false;
bool gbMP_HudShowHelp = false;
bool gbMultiplayerSplash = false;
uint8 gnMP_PauseScreenSelection = 0;
bool gMultiplayerSuperBrakeOnPause = true; // declared in main.h
// defined in Script
#if !defined(FINAL) && !defined(MASTER)
bool gDeveloperFlag = true;
#else
bool gDeveloperFlag = false;
#endif
bool gbMP_StartingScriptsFromLua = false; // gbStartingScriptsFromLua
bool gbMP_DrawHudCars = false;
bool gbMP_RenderNativeEntities = false;

bool gIsMultiplayerGame = false;
CVector gVectorSetInLua = CVector(0.0f, 0.0f, 0.0f);

bool gMultiplayerCheat1 = false;
bool gMultiplayerCheat2 = false;
bool gMultiplayerCheat3 = false;
bool gMultiplayerCheat4 = false;

#ifdef USE_COMPILED_LUA
bool gbIsUsingLUASource = false; // false - compiled (.LC)
#else
bool gbIsUsingLUASource = true; // true - source (.LUA)
#endif

CVector gVecForSinglePlayerScript = CVector(0.0f, 0.0f, 0.0f);

#ifdef DEBUG_MULTIGAME_IMPROVEMENTS
bool gAllowCreateElement = false;
#endif

bool g_bInitMPTime = false; // bss
float g_fMPPrevTime = 0.0f; // vcs bss

const char* luaFiles[] = // usless
{
#ifdef GTA_LIBERTY
    "LUASCRIPTS\\CAPTURETHEFLAG.LUA.LC",
    "LUASCRIPTS\\COMMENTARY.LUA.LC",
    "LUASCRIPTS\\CUTSCENEPLAYER.LUA.LC",
    "LUASCRIPTS\\DEATHMATCH.LUA.LC",
    "LUASCRIPTS\\DEFENDTHEBASE.LUA.LC",
    "LUASCRIPTS\\FREEROAM.LUA.LC",
    "LUASCRIPTS\\GTA.LUA.LC",
    "LUASCRIPTS\\HITPARADE.LUA.LC",
    "LUASCRIPTS\\MAINLOOP.LUA.LC",
    "LUASCRIPTS\\MULTIRACE.LUA.LC",
    "LUASCRIPTS\\PICKUPBLIPS.LUA.LC",
    "LUASCRIPTS\\PICKUPS.LUA.LC",
    "LUASCRIPTS\\POWERUPS.LUA.LC",
    "LUASCRIPTS\\SCORESHEET.LUA.LC",
    "LUASCRIPTS\\SIXTYSECONDS.LUA.LC",
    "LUASCRIPTS\\SPAWN.LUA.LC",
    "LUASCRIPTS\\STDLIB.LUA.LC",
    "LUASCRIPTS\\TANK.LUA.LC"
#else // GTA_MIAMI
    "LUA\\CAPTURETHEFLAG.LUA.LC",
    "LUA\\CAPTURETHEFLAGDATA.LUA.LC",
    "LUA\\COLLECTTHEGOLD.LUA.LC",
    "LUA\\COMMENTARY.LUA.LC",
    "LUA\\COPSANDROBBERS.LUA.LC",
    "LUA\\CTFPICKUPDATA.LUA.LC",
    "LUA\\CUTSCENEPLAYER.LUA.LC",
    "LUA\\DEATHMATCH.LUA.LC",
    "LUA\\DEFENDTHEBASE.LUA.LC",
    "LUA\\DEFENDTHEBASEDATA.LUA.LC",
    "LUA\\DTBPICKUPDATA.LUA.LC",
    "LUA\\FLAGBALL.LUA.LC",
    "LUA\\FLAGBALLDATA.LUA.LC",
    "LUA\\FREEROAM.LUA.LC",
    "LUA\\GAMEMODESKELETON.LUA.LC",
    "LUA\\GTA.LUA.LC",
    "LUA\\HITPARADE.LUA.LC",
    "LUA\\HUNTERATTACK.LUA.LC",
    "LUA\\HUNTERATTACKDATA.LUA.LC",
    "LUA\\IRONMANTRACKDATA.LUA.LC",
    "LUA\\JETSKIRACEDATA.LUA.LC",
    "LUA\\MAINLOOP.LUA.LC",
    "LUA\\MULTIRACE.LUA.LC",
    "LUA\\MULTIRACEPICKUPDATA.LUA.LC",
    "LUA\\MULTIRACETRACKDATA.LUA.LC",
    "LUA\\PICKUPBLIPS.LUA.LC",
    "LUA\\PICKUPS.LUA.LC",
    "LUA\\PICKUPSDATA.LUA.LC",
    "LUA\\POWERUPDATA.LUA.LC",
    "LUA\\POWERUPS.LUA.LC",
    "LUA\\SCORESHEET.LUA.LC",
    "LUA\\SHAREDLIB.LUA.LC",
    "LUA\\SIXTYSECONDS.LUA.LC",
    "LUA\\SIXTYSECONDSDATA.LUA.LC",
    "LUA\\SPAWN.LUA.LC",
    "LUA\\SPAWNPOINTDATA.LUA.LC",
    "LUA\\STDLIB.LUA.LC",
    "LUA\\TANK.LUA.LC",
    "LUA\\TANKDATA.LUA.LC",
    "LUA\\TEAMDEATHMATCH.LUA.LC",
    "LUA\\VIP.LUA.LC",
    "LUA\\VIPDATA.LUA.LC"
#endif
};

#ifdef DEBUG_MULTIGAME
#include "multiplayer/LScript.h"
uint32 nDebugLuaIndex = 0;
uint32 nDebugPedIndex = 0;
uint32 nDebugObjIndex = 7405;
const char* luaDebugFiles[] =
{
    "Test/TEST2.LUA",
    "Test/CONSTTEST.LUA",
};
void DoTest(int mode)
{
	OpenConsole();
    cLWrapper& wrapper = cLWrapper::Instance();
	if(mode == 0)
		cLScript::InitializeDebug(luaDebugFiles[nDebugLuaIndex]); // and do
	else if(mode == 1)
		cLScript::Initialize();
    //cLScript::Shutdown();
    //cLScript::RunMainScript();
}

#ifdef DEBUGMENU
static const char* mgTypes[] = {
#ifdef GTA_LIBERTY
	"Liberty City Survivor [0]",      // DEATHMATCH
#else
	"Vice City Survivor [0]",         // DEATHMATCH
#endif
	"Street Rage [1]",                // MULTIRACE
	"Protection Racket [2]",          // DEFENDTHEBASE
#ifdef GTA_LIBERTY
	"Get Stretch [3]",                // CTF
#else
	"Taken For A Ride [3]",           // CTF
#endif
	"Tanks for the Memories [4]",     // TANK
	"The hit list [5]",               // HITPARADE
#ifdef GTA_LIBERTY
	"The wedding list [6]",           // SIXTYSECONDS
#else
	"Grand Theft Auto [6]",           // SIXTYSECONDS
	"Might Of The Hunter [7]",        // HUNTERATTACK
	"Empire Takedown [8]",            // FLAGBALL
	"Vip Rip [9]"
#endif
};

static const char* mgTypeNames[] = {
	"DEATHMATCH [0]",
	"MULTIRACE [1]",
	"DEFENDTHEBASE [2]",
	"CTF [3]",
	"TANK [4]",
	"HITPARADE [5]",
	"SIXTYSECONDS [6]",
#ifndef GTA_LIBERTY
	"HUNTERATTACK [7]",
	"FLAGBALL [8]",
	"VIP [9]"
#endif
};

void DebugMultiGameMsg(const char* msg)
{
	if (!msg) { return; }
	wchar strW[200];
	AsciiToUnicode(msg, strW);
	TheHud->SetHelpMessage(strW, true);
}
void DebugMenuStartLua() {
	OpenConsole();
	debug("net::packet_id_list_t::snPacketCount: %d\n", net::packet_id_list_t::snPacketCount);
    DMAudio.PlayFrontEndSound(SOUND_PICKUP_ARMOUR, 0);
}

void DebugStartNewNetworkGame() {
	DMAudio.PlayFrontEndSound(SOUND_PICKUP_ARMOUR, 0);
	//gIsMultiplayerGame = true;
	menuOn = false;
	TheLobby.LoadMultiplayer();
	//startNewNetworkGame();
}

void DebugMenuMPDebugCode() {
	bool b = CPad::GetPad(0)->GetLeftShift();
    if(b)
        OpenConsole();
    DoTest(b ? 1 : 0);
    DMAudio.PlayFrontEndSound(SOUND_HUD, 0);
}

void DebugMenuMPTestPackets() {
	OpenConsole();
	gtMP_PacketIDs.TestSizes();
	//assert('VICE' == 0x56494345);
	//assert('GTA3' == 0x47544133);
}

int32 gnMPGameType = (int32)eGameType::DEATHMATCH;
void DebugMenuMPSetGameType() {
	TheMPGame.SetGameType((eGameType)gnMPGameType);
	//char msg[200];
	//sprintf(msg, "%d mp gametype", TheMPGame.GetGameType());
	//DebugMultiGameMsg(msg);
	DebugMultiGameMsg(mgTypes[(int32)TheMPGame.GetGameType()]);
	DMAudio.PlayFrontEndSound(SOUND_PICKUP_ARMOUR, 0);
}
void DebugMenuMPAddPeer() {
	//TheMPGame.GetPlayerList().push_back(new sPeerState(1));
	//TheMPGame.GetPlayerList().push_back(new sPeerState(2));
	DMAudio.PlayFrontEndSound(SOUND_PICKUP_ARMOUR, 0);
}
void DebugMenuMPHud() {
	gIsMultiplayerGame = true;
#if 0
	gbMP_DrawPauseScreen = true;
	if (TheMPGame.GetGameZoneInfo()){
		delete TheMPGame.GetGameZoneInfo();
		TheMPGame.SetGameZoneInfo(nil);
	}
	TheMPGame.SetGameZoneInfo(new cGameZoneInfo(CGame::currLevel == LEVEL_MAINLAND));
	//TheMPGame.SetGameZoneInfo(new cGameZoneInfo(0)); // beach
	//gbMP_DrawPauseScreenNoBox = true;
#else // radar test
	FindPlayerPed()->Teleport({-83.0f, 1065.0f, 11.0f});
	TheMPGame.SetGameZoneInfo(new cGameZoneInfo(0));
#endif
	DMAudio.PlayFrontEndSound(SOUND_PICKUP_ARMOUR, 0);
}
void DebugMenuMPTestPrint() {
	DMAudio.PlayFrontEndSound(SOUND_PICKUP_ARMOUR, 0);
	TheHud->PrintMPLeftActivityZone(0);
	//TheMPGame.SetCTFScoreLimit(1); // tmp test
}
void DebugMenuMPView() {
	DMAudio.PlayFrontEndSound(SOUND_PICKUP_ARMOUR, 0);
	//SpawnPed(MI_HFYBE, ePedType::PEDTYPE_CIVFEMALE);
	static CPed* pPed = nil;
	if (pPed) {
		CWorld::Remove(pPed);
		delete pPed;
		pPed = nil;
	}
	tModelInfoEntryMP* model = &ga_netModelList[nDebugPedIndex];
	int32 mi = 0;
	char buffer[256];
	sprintf(buffer, "Model: %s, Name: %s", model->name, UnicodeToAscii(TheText.Get(model->key)));
	DebugMultiGameMsg(buffer);
	CModelInfo::GetModelInfo(ga_netModelList[nDebugPedIndex].name, &mi);
	pPed = SpawnPed(mi, ePedType::PEDTYPE_CIVFEMALE);
}
void DebugMenuObjView() {
	DMAudio.PlayFrontEndSound(SOUND_PICKUP_ARMOUR, 0);
	static CObject* pObj = nil;
	if (pObj) {
		CWorld::Remove(pObj);
		delete pObj;
		pObj = nil;
	}
	pObj = SpawnObject(nDebugObjIndex);
}
void DebugMenuMPTestCode() {
	DMAudio.PlayFrontEndSound(SOUND_PICKUP_ARMOUR, 0);
	//int32 id = CRadar::SetCoordBlip(eBlipType::BLIP_COORD, CVector(0, 0, 0), 0xFF0000FF, eBlipDisplay::BLIP_DISPLAY_BOTH);
	return;
	OpenConsole();
	//for (int32 i = 0; i < WEAPONTYPE_TOTALWEAPONS; i++) {
	//	debug("%d: 0x%X\n", i, CWeaponInfo::GetWeaponInfo((eWeaponType)i)->m_Flags);
	//}
	//for (int32 i = MI_FIRST_VEHICLE; i < MI_LAST_VEHICLE + 1; i++) {
	//	if (CModelInfo::IsBoatModel(i)) {
	//		tBoatHandlingData* handling = GET_BOAT_HANDLING(i);
	//		if(handling)
	//			mod_HandlingManager.DisplayHandlingData((tHandlingData*)handling, 2);
	//	}
	//}
}
void DebugMenuMPTestStreaming() {
	DebugMultiGameMsg("Test MP Streaming");
	//DMAudio.PlayFrontEndSound(SOUND_PICKUP_ARMOUR, 0);
	gbPrintStreaming = true;
	//gIsMultiplayerGame = true;
	menuOn = false;
	CStreaming::LoadPedbanksForMultiplayer();
	CStreaming::LoadCarbanksForMultiplayer();
	CStreaming::LoadAllRequestedModels(false);
	TheLobby.LoadMultiplayer();
	//TheLobby.HandleHostStartGameState();
	//startNewNetworkGame();
}
void DebugMenuExecLuaInput() {
	OpenConsole();
	if (!gIsMultiplayerGame)
		return;
	char buff[260];
	//DMAudio.PlayFrontEndSound(SOUND_GARAGE_BAD_VEHICLE, 0);
	SetConsoleColor(5);
	printf("Enter LUA String: ");
	fgets(buff, sizeof(buff), stdin);
	SetConsoleColor(6);
	int32 len = strlen(buff);
	if (len > 0 && buff[len - 1] == '\n') {
		buff[len - 1] = '\0';
	}
	//if (cLWrapper::Instance().m_luaVM)
	if (HasLuaInitialized())
		cLWrapper::Instance().ExecString(buff);
	//DMAudio.PlayFrontEndSound(SOUND_PICKUP_PACMAN_PILL, 0);
}

void DebugMenuSwitchScoreBoard() {
	if (!gIsMultiplayerGame)
		return;
	static bool toggle = false;
	toggle ^= true;

	//if (cLWrapper::Instance().m_luaVM)
	if (HasLuaInitialized())
		cLWrapper::Instance().ExecString(toggle ? "main.scores:Show()" : "main.scores:Hide()"); // SCORESHEET.LUA
	DebugMultiGameMsg(toggle ? "main.scores:Show()" : "main.scores:Hide()");
}
void DebugMenuNightMultiGame() {
	//if (!gIsMultiplayerGame)
	//	return;
	CClock::SetGameClock(3, 0);
}
void DebugMenuMultiGameSetError() { // test menu error mg page
	if (!gIsMultiplayerGame)
		return;
	cAdhoc& Adhoc = cAdhoc::Instance();
	//Adhoc.SetExitError();
	Adhoc.SetHasError();
	//base::string reason = "pizda";
	//Adhoc.TerminateWithError(reason);
}
void DebugMenuGivePlayerPickups() {
	if (!gIsMultiplayerGame)
		return;
	DebugMultiGameMsg("ok");
	FindPlayerPed()->m_nPowerups = 0xFF;
}
void DebugMenuSitPas() {
	CPlayerPed* pPlayer = FindPlayerPed();
	CPlayerInfo* pPlayerInfo = &CWorld::Players[CWorld::PlayerInFocus];
	if (!pPlayer || !pPlayerInfo)
		return;

	bool weAreOnBoat = false;
	float lastCloseness = 0.0f;
	CVehicle* carBelow = nil;
	CEntity* surfaceBelow = pPlayer->m_pCurrentPhysSurface;
	if (surfaceBelow && surfaceBelow->IsVehicle()) {
		carBelow = (CVehicle*)surfaceBelow;
		if (carBelow->IsBoat()) {
			weAreOnBoat = true;
			pPlayer->bOnBoat = true;
			if (carBelow->GetStatus() != STATUS_WRECKED && carBelow->GetUp().z > 0.3f)
				pPlayer->SetSeekBoatPosition(carBelow);
		}
	}
	// Find closest car
	if (!weAreOnBoat) {
		float radius = 10.0f;
		float minX = pPlayer->GetPosition().x - radius;
		float maxX = radius + pPlayer->GetPosition().x;
		float minY = pPlayer->GetPosition().y - radius;
		float maxY = radius + pPlayer->GetPosition().y;

		int minXSector = CWorld::GetSectorIndexX(minX);
		if (minXSector < 0) minXSector = 0;
		int minYSector = CWorld::GetSectorIndexY(minY);
		if (minYSector < 0) minYSector = 0;
		int maxXSector = CWorld::GetSectorIndexX(maxX);
		if (maxXSector > NUMSECTORS_X - 1) maxXSector = NUMSECTORS_X - 1;
		int maxYSector = CWorld::GetSectorIndexY(maxY);
		if (maxYSector > NUMSECTORS_Y - 1) maxYSector = NUMSECTORS_Y - 1;

		CWorld::AdvanceCurrentScanCode();

		for (int curY = minYSector; curY <= maxYSector; curY++) {
			for (int curX = minXSector; curX <= maxXSector; curX++) {
				CSector* sector = CWorld::GetSector(curX, curY);
				pPlayerInfo->FindClosestCarSectorList(sector->m_lists[ENTITYLIST_VEHICLES], pPlayer, minX, minY, maxX, maxY, &lastCloseness,
					&carBelow);
				pPlayerInfo->CPlayerInfo::FindClosestCarSectorList(sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], pPlayer, minX, minY, maxX,
					maxY, &lastCloseness, &carBelow);
			}
		}
	}

	// carBelow is now closest vehicle
	if (carBelow && !weAreOnBoat) {

		pPlayer->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, carBelow);
		//carBelow->bHasBeenOwnedByPlayer = true; // NOT STAR from SpawnCar(int id)
		carBelow->AutoPilot.m_fMaxTrafficSpeed = carBelow->AutoPilot.m_nCruiseSpeed = 25;
		carBelow->AutoPilot.m_nTempAction = TEMPACT_NONE;
		//carBelow->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;

		/*if(carBelow->GetStatus() == STATUS_TRAIN_NOT_MOVING) {
				m_pPed->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, carBelow);
		} else if(carBelow->IsBoat()) {
				if(!carBelow->pDriver) {
						m_pPed->m_vehDoor = 0;
						m_pPed->SetEnterCar(carBelow, m_pPed->m_vehDoor);
				}
		} else {
				m_pPed->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, carBelow); // enter into car
				// m_pPed->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, carBelow);
		}*/
	}
}

#include "multiplayer/elements/sElement.h"
#include "multiplayer/elements/sElementPhysical.h"
#include "multiplayer/elements/sPickup.h"
#include "multiplayer/elements/sPlayer.h"
#include "multiplayer/elements/sPed.h"
#include "multiplayer/elements/sSpriteBase.h"
#include "multiplayer/elements/sNetMeter2d.h"
#include "multiplayer/elements/sTextSprite.h"
#include "multiplayer/elements/sRadarBlip.h"
#ifndef GTA_LIBERTY
#include "multiplayer/elements/sBmx.h"
#include "multiplayer/elements/sBoat.h"
#include "multiplayer/elements/sPlane.h"
#include "multiplayer/elements/sHeli.h"
#include "multiplayer/elements/sQuadBike.h"
#endif
#include "multiplayer/elements/sBike.h"
#include "multiplayer/elements/sAutomobile.h"
#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
#include "multiplayer/elements/sObject.h"
#endif

const char* GetElementStringType(sElement* pElement) {
	if (!pElement)
		return nil;

	ElementCapability capability = pElement->GetCapability();

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	if (sObject::Capability() == capability)
		return "sObject";
#endif
#ifndef GTA_LIBERTY
	if (sQuadBike::Capability() == capability)
		return "sQuadBike";
#endif
	if (sAutomobile::Capability() == capability)
		return "sAutomobile";
#ifndef GTA_LIBERTY
	if (sHeli::Capability() == capability)
		return "sHeli";
	if (sPlane::Capability() == capability)
		return "sPlane";
	if (sBoat::Capability() == capability)
		return "sBoat";
	if (sAutomobileBase::Capability() == capability)
		return "sAutomobileBase";
#endif
	if (sBike::Capability() == capability)
		return "sBike";
#ifndef GTA_LIBERTY
	if (sBmx::Capability() == capability)
		return "sBmx";
	if (sNetMeter2d::Capability() == capability)
		return "sNetMeter2d";
#endif
	if (sTextSprite::Capability() == capability)
		return "sTextSprite";
	if (sVehicle::Capability() == capability)
		return "sVehicle";
	if (sPed::Capability() == capability)
		return "sPed";
	if (sRadarBlip::Capability() == capability)
		return "sRadarBlip";
#ifndef GTA_LIBERTY
	if (sSpriteBase::Capability() == capability)
		return "sSpriteBase";
#endif
	if (sElementPhysical::Capability() == capability)
		return "sElementPhysical";
	if (sPlayer::Capability() == capability)
		return "sPlayer";
	if (sPickup::Capability() == capability)
		return "sPickup";
	if (sElement::Capability() == capability)
		return "sElement";

	return nil;
}

const char* GetPhysicalMGStringType(cPhysicalMG* pPhysical)
{
	if (!pPhysical)
		return nil;

	const char* name = "CPhysical";
	if (pPhysical->IsMultiplayer()) {
		name = "cPhysicalMG";
		if(pPhysical->bIsPed)
			name = "cPedMG";
		else if (pPhysical->bIsVehicle) {
			name = "cVehicleMG";
			switch (((cVehicleMG*)pPhysical)->m_vehType)
			{
				case eVehicleType::VEHICLE_TYPE_BIKE:
					name = "cBikeMG";
					break;
				case eVehicleType::VEHICLE_TYPE_CAR:
					name = "cAutomobileMG";
					break;
#ifndef GTA_LIBERTY
				case eVehicleType::VEHICLE_TYPE_BMX:
					name = "cBmxMG";
					break;
				case eVehicleType::VEHICLE_TYPE_BOAT:
				case eVehicleType::VEHICLE_TYPE_JETSKI:
					name = "cBoatMG";
					break;
				case eVehicleType::VEHICLE_TYPE_PLANE:
					name = "cPlaneMG";
					break;
				case eVehicleType::VEHICLE_TYPE_HELI:
					name = "cHeliMG";
					break;
				case eVehicleType::VEHICLE_TYPE_QUAD:
					name = "cQuadBikeMG";
					break;
#endif
			}
		}
#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
		else if (((cPhysicalMG*)pPhysical)->bIsObject)
			name = "cObjectMG";
#endif
	}
	return name;
};

uint32 GetSyncSizeByElement(sElement* pElement) {
	if (!pElement)
		return 0;

	ElementCapability capability = pElement->GetCapability();

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	if (sObject::Capability() == capability)
		return sizeof(sObjectSync);
#endif
#ifndef GTA_LIBERTY
	if (sQuadBike::Capability() == capability)
		return sizeof(sQuadBikeSync);
#endif
	if (sAutomobile::Capability() == capability)
		return sizeof(sAutomobileSync);
#ifndef GTA_LIBERTY
	if (sHeli::Capability() == capability)
		return sizeof(sHeliSync);
	if (sPlane::Capability() == capability)
		return sizeof(sPlaneSync);
	if (sBoat::Capability() == capability)
		return sizeof(sBoatSync);
	if (sAutomobileBase::Capability() == capability)
		return sizeof(sAutomobileBaseSync);
#endif
	if (sBike::Capability() == capability)
		return sizeof(sBikeSync);
#ifndef GTA_LIBERTY
	if (sBmx::Capability() == capability)
		return sizeof(sBmxSync);
	if (sNetMeter2d::Capability() == capability)
		return sizeof(sNetMeter2dSync);
#endif
	if (sTextSprite::Capability() == capability)
		return sizeof(sTextSpriteSync);
	if (sVehicle::Capability() == capability)
		return sizeof(sVehicleSync);
	if (sPed::Capability() == capability)
		return sizeof(sPedSync);
	if (sRadarBlip::Capability() == capability)
		return sizeof(sRadarBlipSync);
#ifndef GTA_LIBERTY
	if (sSpriteBase::Capability() == capability)
		return sizeof(sSpriteBaseSync);
#endif
	if (sElementPhysical::Capability() == capability)
		return sizeof(sElementPhysicalSync);
	if (sPlayer::Capability() == capability)
		return sizeof(sPlayerSync);
	if (sPickup::Capability() == capability)
		return sizeof(sPickupSync);
	if (sElement::Capability() == capability)
		return sizeof(sElementSync);
	 
	return 0;
}

void DumpSectorsList(void)
{
	OpenConsole();

	CPtrNode* node;
	uint32 k = 0;
	for (int32 i = 0; i < NUMSECTORS_Y; i++) {
		for (int32 j = 0; j < NUMSECTORS_X; j++) {
			k = 0;
			CSector* s = &CWorld::ms_aSectors[i][j];
			for (node = s->m_lists[ENTITYLIST_MULTIPLAYER].first; node; node = node->next) {
				debug("%d %d %d %s\n", j, i, k, GetPhysicalMGStringType(((cPhysicalMG*)node->item)));
				++k;
			}
		}
	}
}

void DebugMenuTestElements() {
	//if (!gIsMultiplayerGame)
	//	return;
	TODO(); // normal recheck hier
	OpenConsole();
	void* caps[] = 
	{
		sElement::Capability,
		sPickup::Capability,
		sPlayer::Capability,
		sElementPhysical::Capability,
#ifndef GTA_LIBERTY
		sSpriteBase::Capability,
#endif
		sRadarBlip::Capability,
		sPed::Capability,
		sVehicle::Capability,
		sTextSprite::Capability,
#ifndef GTA_LIBERTY
		sNetMeter2d::Capability,
		sBmx::Capability,
#endif
		sBike::Capability,
#ifndef GTA_LIBERTY
		sAutomobileBase::Capability,
		sBoat::Capability,
		sPlane::Capability,
		sHeli::Capability,
#endif
		sAutomobile::Capability,
#ifndef GTA_LIBERTY
		sQuadBike::Capability,
#endif
#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
		sObject::Capability,
#endif
	};
	for (int32 i = 0; i < ARRAY_SIZE(caps); i++)
	{
		for (int32 j = i + 1; j < ARRAY_SIZE(caps); j++)
		{
			printf("test %d 0x%p - %d 0x%p\n", i, caps[i], j, caps[j]);
			assert(caps[i] != caps[j]);
		}
	}
	printf("OK\n");
}

void TestSyncStream() {
	void OpenConsole();
	OpenConsole();

	char buffer[1024] = { 0 };
	sWriteSyncStream* writeStream = reinterpret_cast<sWriteSyncStream*>(buffer);
	writeStream->Init();

	CVector vec = { 1.0f, 2.0f, 3.0f };
	writeStream->WriteVector(vec);

	CVector2D vec2d = { 4.0f, 5.0f };
	writeStream->WriteVector2D(vec2d);

	base::string str("HA_HNT"); // 6
	writeStream->WriteString(str);

	CRGBA colour = { 10, 20, 30, 40 };
	writeStream->WriteColour(colour);

	int8 i8 = -1;
	writeStream->WriteI8(i8);

	uint8 u8 = 255;
	writeStream->WriteU8(u8);

	int16 i16 = -100;
	writeStream->WriteI16(i16);

	uint16 u16 = 65535;
	writeStream->WriteU16(u16);

	int32 i32 = -100000;
	writeStream->WriteI32(i32);

	uint32 u32 = 4294967295U;
	writeStream->WriteU32(u32);

	float f = 3.14159f;
	writeStream->WriteFloat(f);

	uint8 buff[3] = { 1, 2, 3 };
	writeStream->WriteBuffer(buff, 3);

	uint16 written_length = writeStream->GetLength();
	uint16 data_length = written_length - sizeof(uint16);
	uint8* data = writeStream->GetBuffer();

	writeStream->DumpContents();

	sReadSyncStream readStream;
	readStream.Seekg(data);
	readStream.m_pBufferEnd = data + data_length;

	CVector vec_out;
	readStream.ReadVector(vec_out);

	CVector2D vec2d_out;
	readStream.ReadVector2D(vec2d_out);

	base::string str_out;
	readStream.ReadString(str_out);

	CRGBA colour_out;
	readStream.ReadColour(colour_out);

	int8 i8_out;
	readStream.ReadI8(i8_out);

	uint8 u8_out;
	readStream.ReadU8(u8_out);

	int16 i16_out;
	readStream.ReadI16(i16_out);

	uint16 u16_out;
	readStream.ReadU16(u16_out);

	int32 i32_out;
	readStream.ReadI32(i32_out);

	uint32 u32_out;
	readStream.ReadU32(u32_out);

	float f_out;
	readStream.ReadFloat(f_out);

	uint8 buff_out[3];
	readStream.ReadBuffer(buff_out, 3);

	readStream.DumpContents();

	bool passed = true;
	if (!readStream.IsEmpty()) {
		printf("Buffer not fully read: remaining %zu bytes\n", readStream.m_pBufferEnd - readStream.m_pBuffer);
		passed = false;
	}
	if (vec.x != vec_out.x || vec.y != vec_out.y || vec.z != vec_out.z) {
		printf("CVector mismatch: original (%.2f, %.2f, %.2f), read (%.2f, %.2f, %.2f)\n",
			vec.x, vec.y, vec.z, vec_out.x, vec_out.y, vec_out.z);
		passed = false;
	}
	if (vec2d.x != vec2d_out.x || vec2d.y != vec2d_out.y) {
		printf("CVector2D mismatch: original (%.2f, %.2f), read (%.2f, %.2f)\n",
			vec2d.x, vec2d.y, vec2d_out.x, vec2d_out.y);
		passed = false;
	}
	if (str.length() != str_out.length() || strcmp(str.c_str(), str_out.c_str()) != 0) {
		printf("String mismatch: original '%s', read '%s'\n", str.c_str(), str_out.c_str());
		passed = false;
	}
	if (colour.red != colour_out.red || colour.green != colour_out.green ||
		colour.blue != colour_out.blue || colour.alpha != colour_out.alpha) {
		printf("CRGBA mismatch: original (%u,%u,%u,%u), read (%u,%u,%u,%u)\n",
			colour.red, colour.green, colour.blue, colour.alpha,
			colour_out.red, colour_out.green, colour_out.blue, colour_out.alpha);
		passed = false;
	}
	if (i8 != i8_out) {
		printf("int8 mismatch: original %d, read %d\n", i8, i8_out);
		passed = false;
	}
	if (u8 != u8_out) {
		printf("uint8 mismatch: original %u, read %u\n", u8, u8_out);
		passed = false;
	}
	if (i16 != i16_out) {
		printf("int16 mismatch: original %d, read %d\n", i16, i16_out);
		passed = false;
	}
	if (u16 != u16_out) {
		printf("uint16 mismatch: original %u, read %u\n", u16, u16_out);
		passed = false;
	}
	if (i32 != i32_out) {
		printf("int32 mismatch: original %d, read %d\n", i32, i32_out);
		passed = false;
	}
	if (u32 != u32_out) {
		printf("uint32 mismatch: original %u, read %u\n", u32, u32_out);
		passed = false;
	}
	if (fabs(f - f_out) > 1e-6f) {
		printf("float mismatch: original %.6f, read %.6f\n", f, f_out);
		passed = false;
	}
	if (memcmp(buff, buff_out, 3) != 0) {
		printf("Buffer mismatch: original [%u,%u,%u], read [%u,%u,%u]\n",
			buff[0], buff[1], buff[2], buff_out[0], buff_out[1], buff_out[2]);
		passed = false;
	}
	if (passed)
		printf("Test passed\n");
	else
		printf("Test failed\n");
}
uint8* DebugLoadPSPDump(const char* path, int32* nOutSize) {
	int32 fd = CFileMgr::OpenFile(path, "rb");
	if (!fd)
		return nil;

	uint32 fsize = CFileMgr::GetFileSize(fd);
	if (nOutSize) *nOutSize = fsize;
	uint8* pData = new uint8[fsize + 1];
	CFileMgr::Read(fd, (const char*)pData, fsize);
	CFileMgr::CloseFile(fd);
	return pData;
}
void DebugRebasePSPDump(uint8* pData, uint32 nSize, uint32 nAddr) { // update pointer at address from psp to pc
	uint32* pUpdptr = (uint32*)DebugFixupPSPDump(pData, nSize, nAddr);
	*(uintptr*)pUpdptr = (uintptr)DebugFixupPSPDump(pData, nSize, *pUpdptr); // !! warn! in x64 build x32 bin owerride next 4b
}
uint8* DebugFixupPSPDump(uint8* pData, uint32 nSize, uint32 nAddr) { // get real pointer from psp
	const uint32_t rambase = 0x08800000; // psp
#define PTR(addr) ((uint8*)( ((uint32)(addr) < rambase || (size_t)((uint32)(addr) - rambase) >= nSize) ? \
                        nil : (void*)(pData + (size_t)((uint32)(addr) - rambase)) ))
#define UNPTR(localptr) ((uint32)(( (uint8*)(localptr) >= pData && (size_t)((uint8*)(localptr) - pData) < nSize ) \
                        ? (rambase + (uint32)((size_t)((uint8*)(localptr) - pData))) : 0))
#define OFF(_base, off, type) ((type)&(((uint8*)_base)[off]) )
	assert(pData);
	return nAddr ? PTR(nAddr) : nil;
#undef PTR
#undef UNPTR
#undef OFF

}
void DebugFreePSPDump(uint8* dump) {
	delete[] dump;
}
void TestSyncStream2() {
	OpenConsole();
	int32 nSize = 0;
	uint8* dump = DebugLoadPSPDump("MPDumps\\RAMCNET.dump", &nSize);
	if (!dump) { debug("DebugFreePSPDump blya\n"); return; }
	debug("psp: 0x%p\n", dump);
	assert(dump);
	uint32 nAddr = 0x0BFB6800; // cNet2dMeterSync ReadStream
	sReadSyncStream* stream = (sReadSyncStream*)DebugFixupPSPDump(dump, nSize, nAddr); // https://prnt.sc/WRCMpBIGYUGc
	debug("stream: 0x%p\n", stream);
	//DebugMenuExecLuaInput(); // lazy wait
#ifdef _WIN64
	//DebugRebasePSPDump(dump, nSize, nAddr + 8);
	//*(uintptr*)(((uint8*)stream) + 16) = *(uintptr*)(((uint8*)stream) + 8);
	//DebugRebasePSPDump(dump, nSize, nAddr + 4);
	//*(uintptr*)(((uint8*)stream) + 8) = *(uintptr*)(((uint8*)stream) + 4);
	//DebugRebasePSPDump(dump, nSize, nAddr);
	//// no need shift

#define REBASE_AND_SHIFT(pStream, offset) \
        DebugRebasePSPDump(dump, nSize, nAddr + offset); \
        *(uintptr_t*)(((uint8*)pStream) + (offset * 2)) = \
            *(uintptr_t*)(((uint8*)pStream) + offset);

	REBASE_AND_SHIFT(stream, 8);
	REBASE_AND_SHIFT(stream, 4);
	REBASE_AND_SHIFT(stream, 0);

#undef REBASE_AND_SHIFT
#else
	DebugRebasePSPDump(dump, nSize, nAddr);
	DebugRebasePSPDump(dump, nSize, nAddr + 4);
	DebugRebasePSPDump(dump, nSize, nAddr + 8);
#endif
	debug("sync: 0x%p\n", stream->m_pBuffer);
	stream->DumpContents();
	debug("diffmask sNetMeter2d: 0x%X\n", stream->ReadU8());
	debug("fakinpad sNetMeter2d: 0x%X\n", stream->ReadU8());
	debug("diffmask sSpriteBase: 0x%X\n", stream->ReadU8());
	debug("fakinpad sSpriteBase: 0x%X\n", stream->ReadU8());
	CVector2D pos = stream->ReadVector2D();
	debug("sSpriteBase pos: %f %f\n", pos.x, pos.y); // &1
	CVector2D m_Size = stream->ReadVector2D();
	debug("sSpriteBase m_Size: %f %f\n", m_Size.x, m_Size.y); // &2 &4
	CRGBA col = stream->ReadColour();
	debug("sSpriteBase col: %d %d %d %d\n", col.r, col.g, col.b, col.a); // &8
	debug("sSpriteBase m_nOrder: 0x%X\n", stream->ReadU32()); // &10
	DebugFreePSPDump(dump);
}
struct CPtrNode_bin
{
	uint32 item; // CEntity
	uint32 prev; // CPtrNode
	uint32 next; // CPtrNode
};
struct CPtrList_bin
{
	uint32 first; // CPtrNode_bin
};
struct CSector_bin // (14) todo m_empireList  m_empireOverlapList
{
	CPtrList_bin m_buildingList;
	CPtrList_bin m_buildingOverlapList;
	CPtrList_bin m_multiplayerList;
	CPtrList_bin m_objectList;
	CPtrList_bin m_objectOverlapList;
	CPtrList_bin empire;
	CPtrList_bin empireover;
	CPtrList_bin m_vehicleList;
	CPtrList_bin m_vehicleOverlapList;
	CPtrList_bin m_pedList;
	CPtrList_bin m_pedOverlapList;
	CPtrList_bin unk3;
	CPtrList_bin m_dummyList;
	CPtrList_bin m_dummyOverlapList;
};

CSector_bin* GetSectorByPos(float wx, float wy, CSector_bin* CWorld_ms_aSectors) {
	int32 sectorX = (int32)((wx - WORLD_MIN_X) / SECTOR_SIZE_X);
	int32 sectorY = (int32)((wy - WORLD_MIN_Y) / SECTOR_SIZE_Y);
	if (sectorX < 0) sectorX = 0;
	if (sectorX >= NUMSECTORS_X) sectorX = NUMSECTORS_X - 1;
	if (sectorY < 0) sectorY = 0;
	if (sectorY >= NUMSECTORS_Y) sectorY = NUMSECTORS_Y - 1;
	return &CWorld_ms_aSectors[(NUMSECTORS_X * sectorY) + sectorX];
}
void DumpSector(uint8* dump, uint32 nSize, CSector_bin* pS) {
	// m_pedList
	CPtrNode_bin* pFNode = (CPtrNode_bin*)DebugFixupPSPDump(dump, nSize, pS->m_pedList.first);
	if (pS->m_pedList.first && pFNode->item) {
		debug("m_pedList pFNode: 0x%p  %X\n", pFNode, pS->m_pedList.first);
		while (pFNode)
		{
			uint8* pEntity = DebugFixupPSPDump(dump, nSize, pFNode->item);
			debug("pEnt: %X 0x%p  vft 0x%X, mi %d\n", pFNode->item, pEntity, *(uint32*)(pEntity + 0x5C), *(int16*)(pEntity + 0x56)); // CPlayerPed
			pFNode = (CPtrNode_bin*)DebugFixupPSPDump(dump, nSize, pFNode->next);
		}
	}

	// m_multiplayerList
	pFNode = (CPtrNode_bin*)DebugFixupPSPDump(dump, nSize, pS->m_multiplayerList.first);
	if (pS->m_multiplayerList.first && pFNode->item) {
		debug("m_multiplayerList pFNode: 0x%p  %X\n", pFNode, pS->m_multiplayerList.first);
		while (pFNode)
		{
			uint8* pEntity = DebugFixupPSPDump(dump, nSize, pFNode->item);
			debug("pEnt: %X 0x%p  vft 0x%X, mi %d\n", pFNode->item, pEntity, *(uint32*)(pEntity + 0x5C), *(int16*)(pEntity + 0x56)); // cAutomobileMG
			pFNode = (CPtrNode_bin*)DebugFixupPSPDump(dump, nSize, pFNode->next);
		}
	}

	// m_dummyList
	pFNode = (CPtrNode_bin*)DebugFixupPSPDump(dump, nSize, pS->m_dummyList.first);
	if (pS->m_dummyList.first && pFNode->item) {
		debug("m_dummyList pFNode: 0x%p  %X\n", pFNode, pS->m_dummyList.first);
		while (pFNode)
		{
			uint8* pEntity = DebugFixupPSPDump(dump, nSize, pFNode->item);
			debug("pEnt: %X 0x%p  vft 0x%X, mi %d\n", pFNode->item, pEntity, *(uint32*)(pEntity + 0x5C), *(int16*)(pEntity + 0x56)); // cAutomobileMG
			pFNode = (CPtrNode_bin*)DebugFixupPSPDump(dump, nSize, pFNode->next);
		}
	}

	// m_dummyOverlapList
	pFNode = (CPtrNode_bin*)DebugFixupPSPDump(dump, nSize, pS->m_dummyOverlapList.first);
	if (pS->m_dummyOverlapList.first && pFNode->item) {
		debug("m_dummyOverlapList pFNode: 0x%p  %X\n", pFNode, pS->m_dummyOverlapList.first);
		while (pFNode)
		{
			uint8* pEntity = DebugFixupPSPDump(dump, nSize, pFNode->item);
			debug("pEnt: %X 0x%p  vft 0x%X, mi %d\n", pFNode->item, pEntity, *(uint32*)(pEntity + 0x5C), *(int16*)(pEntity + 0x56)); // cAutomobileMG
			pFNode = (CPtrNode_bin*)DebugFixupPSPDump(dump, nSize, pFNode->next);
		}
	}
}
void TestPSPRAM() {
	OpenConsole();
	int32 nSize = 0;
	uint8* dump = DebugLoadPSPDump("MPDumps\\RAMMPSEC.dump", &nSize);
	if (!dump) { debug("TestPSPRAM blya\n"); return; }
	debug("psp: 0x%p\n", dump);
	assert(dump);
	uint32 nAddr = 0x09929750; // CPlayerPed
	uint32 CWorld_ms_aSectors = 0x09678890; // 08BAB9C8 // array pointer to sector 50*50
	CSector_bin* pSectors = (CSector_bin*)DebugFixupPSPDump(dump, nSize, CWorld_ms_aSectors);
	debug("pSectors: 0x%p\n", pSectors);
	CSector_bin* nSecAddr = GetSectorByPos(502.324f, -161.833f, pSectors);
	debug("s: 0x%p\n", nSecAddr);
	debug("s===: 0x%X\n", (((int8*)nSecAddr) - ((int8*)pSectors)) + CWorld_ms_aSectors + 36);

	//DumpSector(dump, nSize, nSecAddr);

	for (int32 x = 0; x < NUMSECTORS_X; x++)
		for (int32 y = 0; y < NUMSECTORS_Y; y++) {
			CSector_bin(*sectors)[NUMSECTORS_Y] = (CSector_bin(*)[NUMSECTORS_Y])pSectors;
			DumpSector(dump, nSize, &sectors[x][y]);
		}

	uint32* pEs = (uint32*)DebugFixupPSPDump(dump, nSize, 0x08E76AC0); // _ZN9CRenderer21ms_aVisibleEntityPtrsE
	uint32 ms_nNoOfVisibleEntities = *(uint32*)DebugFixupPSPDump(dump, nSize, 0x08BB44B0);
	for (uint32 i = 0; i < ms_nNoOfVisibleEntities; i++)
	{
		uint8* pEntity = DebugFixupPSPDump(dump, nSize, pEs[i]);
		debug("pEnt: %X 0x%p  vft 0x%X, mi %d\n", pEs[i], pEntity, *(uint32*)(pEntity + 0x5C), *(int16*)(pEntity + 0x56)); // cAutomobileMG
	}

	DebugFreePSPDump(dump);
}

int32 gMPDebugPeer = 0;
int32 gMPDebugGroup = BROADCAST_PEER_GROUPID;
TWEAKSWITCHN(gMPDebugPeer, -10, 10, nil, nil, "gMPDebugPeer");
TWEAKSWITCHN(gMPDebugGroup, -10, 10, nil, nil, "gMPDebugGroup");
void DebugMenuSpreadMultigameCrash() {
	// todo, sometimes disconnect
	cMultiGame& Game = cMultiGame::Instance();
	sWriteSyncStream stream;
	net::pckt_game_state& packet = *(net::pckt_game_state*)&stream;
	packet.pckt_size = sizeof(net::pckt_game_state);
	packet.pckt_id = gtMP_PacketIDs.game_state.pckt_id;
	packet.sequence = 0x80;
	packet.zone = -1;
	stream.WriteU16(Game.m_pNetSession->m_nCurTime); // basis
	stream.WriteU8(0); // ack count
	stream.WriteU16(eElementID::MG_ELEMENT_PLAYER_ID | 0x8000); // entity id (sPlayer, DeltaSync)
	stream.WriteU32(0xFFFFFFFF); // delta sync data
	stream.WriteU32(0xFFFFFFFF); // delta sync data
	Game.SendMessage(packet, gMPDebugGroup);
}

void DebugMenuSpreadMultigameDisconnect() {
	cMultiGame& Game = cMultiGame::Instance();
	sWriteSyncStream stream;
	net::pckt_game_state& packet = *(net::pckt_game_state*)&stream;
	packet.pckt_size = sizeof(net::pckt_game_state);
	packet.pckt_id = gtMP_PacketIDs.game_state.pckt_id;
	packet.sequence = 0x80;
	packet.zone = -1;
	stream.WriteU16(Game.m_pNetSession->m_nCurTime); // basis
	stream.WriteU8(0); // ack count
	stream.WriteU16((0xFFFF & 0x7FFFU)); // entity id (wrong id, disconnect, no create flag)
	Game.SendMessage(packet, gMPDebugGroup);
}

void DebugMenuRequestKickPlayer() {
	//MultigameRequestKickPlayer(gMPDebugPeer); // lcs
	MultigameKickPlayer(gMPDebugPeer);
}

void DebugMenuKillAllPlayers() {
	cMultiGame& Game = cMultiGame::Instance();
	if (!gIsMultiplayerGame || Game.m_pNetSession == nil) return;
	net::pckt_kill_player_ped packet{};
	packet.pckt_size = sizeof(net::pckt_kill_player_ped);
	packet.pckt_id = gtMP_PacketIDs.kill_player_ped.pckt_id;
#ifdef GTA_LIBERTY
	for (int32 i = 0; i < Game.m_vPlayers.size(); i++)
	{
		sPlayer* pPlayer = Game.GetPlayer(i);
#else
	cPeerManager& PeerMgr = PeerManager;
	for (int32 i = 0; i < PeerMgr.m_vPlayers.size(); i++)
	{
		sPeerState* peer = PeerMgr.GetPeerAt(i);
		sPlayer* pPlayer = Game.GetPlayer(peer->m_nID);
#endif
		sPlayer* pSelfPlayer = Game.GetPlayer(MP_HOST_INDEX);
		if (pPlayer == pSelfPlayer || !pPlayer) continue;
		packet.player_id = peer->m_nID;
		Game.SendMessagePriority(packet, packet.player_id);
		//Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID); // append packet to queue
		//Game.m_pNetSession->UpdateSend(); // pdp spread buffer // cMultiGame::PerformInitialConnection()
	}
}

void DebugMenuKickAllPlayers() {
	cMultiGame& Game = cMultiGame::Instance();
	if (!gIsMultiplayerGame || Game.m_pNetSession == nil) return;
#ifdef GTA_LIBERTY
	for (int32 i = 0; i < Game.m_vPlayers.size(); i++)
	{
		sPlayer* pPlayer = Game.GetPlayer(i);
#else
	cPeerManager& PeerMgr = PeerManager;
	for (int32 i = 0; i < PeerMgr.m_vPlayers.size(); i++)
	{
		sPeerState* peer = PeerMgr.GetPeerAt(i);
		sPlayer* pPlayer = Game.GetPlayer(peer->m_nID);
#endif
		sPlayer* pSelfPlayer = Game.GetPlayer(MP_HOST_INDEX);
		if (pPlayer == pSelfPlayer || !pPlayer) continue;
		MultigameKickPlayer(peer->m_nID);
	}
}

void DebugMenuTestDeltaSizes() {
	if (!gIsMultiplayerGame) return;
	CBike* b = new CBike(MI_PCJ600, RANDOM_VEHICLE);
	sBike* pElem = new sBike(b);
	sWriteSyncStream stream;
	uint32 beforeSync = stream.pckt_size;
	pElem->WriteSyncToStream(&stream, 0, 0);
#ifdef DEBUG_MULTIGAME
	SetConsoleColor(2);
	debug("!!!! FULL SYNC FOR %s %d  0x%X bytes\n", GetElementStringType(pElem), stream.pckt_size - beforeSync, stream.pckt_size - beforeSync);
	SetConsoleColor(6);
#endif
	//delete pElem;
	//delete b;
}

void DebugMenuTestNetMessages() {
	cMultiGame& Game = cMultiGame::Instance();
	net::pckt_start_fire packet;
	packet.pckt_size = sizeof(net::pckt_start_fire);
	packet.pckt_id = gtMP_PacketIDs.start_fire.pckt_id;
	packet.pos = CVector(0.0f, 0.0f, 0.0f);
	packet.strength = 1.0f;
	packet.propagation = 1;
	packet.source = -1;
	packet.entity = -1;
	Game.SendMessage(packet, BROADCAST_PEER_GROUPID);
}

void DebugMenuTestNetMessagesPri() {
	cMultiGame& Game = cMultiGame::Instance();
	net::pckt_start_fire packet;
	packet.pckt_size = sizeof(net::pckt_start_fire);
	packet.pckt_id = gtMP_PacketIDs.start_fire.pckt_id;
	packet.pos = CVector(0.0f, 0.0f, 0.0f);
	packet.strength = 1.0f;
	packet.propagation = 1;
	packet.source = -1;
	packet.entity = -1;
	Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
}

TWEAKBOOL(gIsMultiplayerGame);
TWEAKBOOL(gDeveloperFlag);
TWEAKSWITCHN(gnMPGameType, 0, (int32)eGameType::NUM_MULIT_GAME_TYPES - 1, mgTypes, nil, "Game Type");
TWEAKSWITCHN(gnMPGameType, 0, (int32)eGameType::NUM_MULIT_GAME_TYPES - 1, mgTypeNames, nil, "Game Type");
//TWEAKSWITCHN(gnMPGameType, 0, (int32)eGameType::NUM_MULIT_GAME_TYPES - 1, nil, nil, "Game Type");
TWEAKFUNCN(DebugMenuMPSetGameType, "Set Game Type");
TWEAKSWITCHN(nDebugPedIndex, 0, MAX_MP_MODELS - 1, nil, nil, "Ped Type");
TWEAKFUNCN(DebugMenuMPView, "Look Ped");
TWEAKSWITCHN(nDebugObjIndex, 0, 10000, nil, nil, "Obj");
TWEAKFUNCN(DebugMenuObjView, "Look Obj");
TWEAKBOOL(gbMP_StartingScriptsFromLua);
TWEAKBOOL(gbIsUsingLUASource);
TWEAKBOOL(gbMP_DrawPauseScreen);
TWEAKBOOL(gbMP_DrawPauseScreenNoBox);
TWEAKBOOL(gbMP_RenderHudExtras);
TWEAKBOOL(gbMP_HudShowHelp);
TWEAKBOOL(gbMultiplayerSplash);
TWEAKBOOL(gMultiplayerSuperBrakeOnPause);
TWEAKBOOL(gbMP_DrawHudCars);
TWEAKBOOL(gbMP_RenderNativeEntities);
TWEAKBOOL(gMultiplayerCheat1);
TWEAKBOOL(gMultiplayerCheat2);
TWEAKBOOL(gMultiplayerCheat3);
TWEAKBOOL(gMultiplayerCheat4);
TWEAKUINT8(gnMP_PauseScreenSelection, 0, 100, 1);
TWEAKSWITCHN(gMPNetDebugLogLevel, 0, (int32)LogLevel::LLEVEL_COUNT - 1, LogLevelNames, nil, "Net Log Level");
#ifdef DEBUG_MULTIGAME
bool gbMultigameDoTimeoutStuff = true;
TWEAKBOOL(gbMultigameDoTimeoutStuff);
#endif

//TWEAKUINT32N(nDebugLuaIndex, 0, ARRAY_SIZE(luaFiles) - 1, 1, "LUA ID");
//TWEAKSWITCHN(nDebugLuaIndex, 0, ARRAY_SIZE(luaFiles) - 1, luaFiles, nil, "LUA");
TWEAKSWITCHN(nDebugLuaIndex, 0, ARRAY_SIZE(luaDebugFiles) - 1, luaDebugFiles, nil, "DEBUG LUA");
TWEAKFUNCN(DebugMenuMultiGameSetError, "SETERROR");
TWEAKFUNCN(TestPSPRAM, "TestPSPRAM");
TWEAKFUNCN(DebugMenuMPDebugCode, "Run Test Lua");
TWEAKFUNCN(DebugMenuStartLua, "Debug Code 2");
TWEAKFUNCN(DebugMenuMPTestCode, "DebugMenuMPTestCode");
TWEAKFUNCN(DebugMenuMPHud, "Test MultiGame Hud");
TWEAKFUNCN(DebugMenuMPTestPrint, "DebugMenuMPTestPrint");
TWEAKFUNCN(DebugMenuMPTestStreaming, "Test MultiGame Streaming");
TWEAKFUNCN(DebugStartNewNetworkGame, "Start Network Game");
TWEAKFUNCN(DebugMenuSwitchScoreBoard, "Switch scoreboard");
TWEAKFUNCN(DebugMenuGivePlayerPickups, "Dat plushki igroku");
TWEAKSWITCHN(gMPDebugPrintLevel, 0, 100, nil, nil, "Debug Print Mode");
TWEAKFUNCN(DebugMenuSitPas, "Sit Pas");
TWEAKFUNCN(DebugMenuNightMultiGame, "Night MG");
TWEAKFUNCN(DebugMenuMPTestPackets, "Test packets sizeofs");
TWEAKFUNCN(DebugMenuExecLuaInput, "Exec lua string"); // todo gui IO? will be noisee
TWEAKFUNCN(DebugMenuTestElements, "Capability test");
TWEAKFUNCN(TestSyncStream, "Stream test");
TWEAKFUNCN(TestSyncStream2, "test Sync parse");
TWEAKFUNCN(DumpSectorsList, "DumpSectorsList");
TWEAKFUNCN(DebugMenuSpreadMultigameCrash, "DebugMenuSpreadMultigameCrash");
TWEAKFUNCN(DebugMenuSpreadMultigameDisconnect, "DebugMenuSpreadMultigameDisconnect");
TWEAKFUNCN(DebugMenuRequestKickPlayer, "DebugMenuRequestKickPlayer");
TWEAKFUNCN(DebugMenuKillAllPlayers, "DebugMenuKillAllPlayers");
TWEAKFUNCN(DebugMenuKickAllPlayers, "DebugMenuKickAllPlayers");
TWEAKFUNCN(DebugMultigameTriggerError, "DebugMultigameTriggerError"); // debug point
TWEAKFUNCN(DebugMenuTestDeltaSizes, "DebugMenuTestDeltaSizes");
TWEAKFUNCN(DebugMenuTestNetMessages, "DebugMenuTestNetMessages");
TWEAKFUNCN(DebugMenuTestNetMessagesPri, "DebugMenuTestNetMessagesPri");
#endif
#endif

//int32 gMPNetDebugLogLevel = LogLevel::LVERBOSE;
//int32 gMPNetDebugLogLevel = LogLevel::LDEBUG;
int32 gMPNetDebugLogLevel = LogLevel::LNOTICE;

#ifndef MASTER
int32 gMPDebugPrintLevel = -1;
void lsn_simsch_debug_render();

void AdhocLobbyPrintDebugStuff()
{
	cAdhoc& Adhoc = cAdhoc::Instance();
	CFont::SetFontStyle(FONT_STANDARD);
	CFont::SetBackgroundOff();
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetWrapx(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetScale(0.85f, 1.0f);
	CFont::SetCentreOff();
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetJustifyOff();
	//CFont::SetColor(CRGBA(240, 20, 20, 255));
	CFont::SetColor(CRGBA(39, 152, 7, 255)); // green
	CFont::SetDropShadowPosition(1);
	CFont::SetPropOn();
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));

	const CRGBA colGreen(39, 152, 7, 255);
	const CRGBA colRed(240, 20, 20, 255);

	float x = 16.0f;
	float y = 120.0f;
	const float ystep = 20.0f;

	char line[512];
	wchar wline[512];

	snprintf(line, sizeof(line), "[ADHOC LOBBY] slots=%d", MP_NUM_MATCHING_GROUPS);
	AsciiToUnicode(line, wline);
	CFont::SetColor(CRGBA(240, 240, 240, 255));
	CFont::PrintString(x, y, wline);
	y += ystep;

	tLobbyRemoteInfo* pEntry = &Adhoc.m_aMatchingInfoRecv->entry;
	if (!pEntry) return;

	for (int32 index = 0; index < MP_NUM_MATCHING_GROUPS; ++index) {
		tAdhocMatchingData& slot = Adhoc.m_aMatchingInfoRecv[index];

		char macBuf[64] = { 0 };
		//tMacAddr addr = slot.addr;
		tMacAddr addr = pEntry->m_nPeersConnInfo[index].macAddr;
		addr.ToString(macBuf);
		bool isBroadcast = addr.IsBroadcast();

		const char* name = "[unknown]";
		//for (int32 p = 0; p < MP_NUM_PEERS; ++p) {
		//	if (Adhoc.m_aMatchingPlayersInfo[p].m_PlayerMacAddr == slot.addr) {
		//		Adhoc.m_aMatchingPlayersInfo[p].m_szPlayerNickname[ADHOCCTL_NICKNAME_LEN - 1] = '\0';
		//		name = Adhoc.m_aMatchingPlayersInfo[p].m_szPlayerNickname;
		//		break;
		//	}
		//}

		base::string strName;
		Adhoc.GetPlayerNameFromMacAddr(strName, addr);
		name = strName.c_str();

		// State -> string
		const char* stateStr = "UNKNOWN";
		switch (slot.nState) {
		case ADHOC_PEER_DISCONNECTED: stateStr = "DISCONNECTED"; break;
		case ADHOC_PEER_PENDING:      stateStr = "PENDING";      break;
		case ADHOC_PEER_HELLOED:      stateStr = "HELLOED";      break;
		case ADHOC_PEER_ACCEPTED:     stateStr = "ACCEPTED";     break;
		case ADHOC_PEER_SELECTED:     stateStr = "SELECTED";     break;
		case ADHOC_PEER_JOINED:       stateStr = "JOINED";       break;
		case ADHOC_PEER_LEAVING:      stateStr = "LEAVING";      break;
		default:                      stateStr = "UNK";         break;
		}

		//CFont::SetColor(isBroadcast ? colRed : colGreen);
		CFont::SetColor(slot.nState == ADHOC_PEER_DISCONNECTED ? colRed : colGreen);

		snprintf(line, sizeof(line), "#%02d: %s - %s state: %s", index, macBuf, name ? name : "[unknown]", stateStr);
		AsciiToUnicode(line, wline);
		CFont::PrintString(x, y, wline);
		y += ystep;

		if (y > SCREEN_SCALE_Y(DEFAULT_SCREEN_HEIGHT) - 30.0f)
			break;
	}
}

void PrintDebugLobbyStuff()
{
	AdhocLobbyPrintDebugStuff();

	CFont::SetFontStyle(FONT_STANDARD);
	CFont::SetBackgroundOff();
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetWrapx(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetScale(0.85f, 1.0f);
	CFont::SetCentreOff();
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetJustifyOff();
	if((((float)mp_time_now_d()) - cLobby::ms_fJoinPrevTime) >= MAX_LOBBY_WAIT_DELAY)
		CFont::SetColor(CRGBA(240, 20, 20, 255));
	else
		CFont::SetColor(CRGBA(39, 152, 7, 255)); // green
	CFont::SetDropShadowPosition(1);
	CFont::SetPropOn();
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));

	float x = 16.0f;
	float y = 90.0f;

	char line[512];
	wchar wline[512];

	sprintf(line, "NOW %f, PREV %f, DELTA %f", (float)mp_time_now_d(), cLobby::ms_fJoinPrevTime, ((float)mp_time_now_d()) - cLobby::ms_fJoinPrevTime);

	AsciiToUnicode(line, wline);
	CFont::PrintString(x, y, wline);
}

void PrintDebugMPProcessStuff()
{
	CFont::SetFontStyle(FONT_STANDARD);
	CFont::SetBackgroundOff();
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetWrapx(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetScale(0.85f, 1.0f);
	CFont::SetCentreOff();
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetJustifyOff();
	if ((((float)mp_time_now_d()) - g_fMPPrevTime) >= MULTI_TIME_OUT_4)
		CFont::SetColor(CRGBA(240, 20, 20, 255));
	else
		CFont::SetColor(CRGBA(39, 152, 7, 255)); // green
	CFont::SetDropShadowPosition(1);
	CFont::SetPropOn();
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));

	float x = 16.0f;
	float y = 200.0f;

	char line[512];
	wchar wline[512];

	sprintf(line, "NOW %f, PREV %f, DELTA %f NSCT %d", (float)mp_time_now_d(), g_fMPPrevTime, ((float)mp_time_now_d()) - g_fMPPrevTime,
		TheMPGame.m_pNetSession ? TheMPGame.m_pNetSession->m_nCurTime : 0);

	AsciiToUnicode(line, wline);
	CFont::PrintString(x, y, wline);
}

void MPPrintDebugStuff()
{
	//if (gIsMultiplayerGame) mp_game_draw_debug_zones();
	//if (gIsMultiplayerGame) lsn_simsch_debug_render();
	return;

	if (FrontEndMenuManager->GetIsMenuActive() && FrontEndMenuManager->IsMPPageActive()) {
		PrintDebugLobbyStuff();
#if !defined(GTA_PSP)
		//AdhocEmuPrintDebugStuff();
#endif
	}
	//return;


	if (!gIsMultiplayerGame)
		return;

	mp_game_draw_debug_net();
	mp_game_draw_debug_zones();
	if (TheMPGame.m_pNetSession)
		TheMPGame.m_pNetSession->PrintNSDebugStuff();
	PrintDebugMPProcessStuff();

	//if (gbPrintSimsch)
	if (gMPDebugPrintLevel == 0)
		lsn_simsch_debug_render();
	else if (gMPDebugPrintLevel == 1)
		PeerManager.PrintDebugStuff();
	else if (gMPDebugPrintLevel == 2)
		CStreaming::PrintStreamingBufferState();

	if (gMPDebugPrintLevel) {
#ifndef MASTER
		AdhocPrintDebugStuff();
//#if !defined(GTA_PSP)
//		AdhocEmuPrintDebugStuff();
//#endif
#endif
	}
}
#endif

struct {
	AnimationId nAnimID[6];
	uint32 nAnimHashes[6];
}
 animDebug = {
	 {
		 ANIM_STD_WALK,
		 ANIM_STD_IDLE,
		 ANIM_STD_RUNSTOP1,
		 ANIM_STD_RUNSTOP2,
		 ANIM_STD_IDLE_CAM,
		 ANIM_STD_CHAT,
	},
	{ 
		0x65766572,
		0x64657372,
		0x20796220,
		0x615A614D,
		0x614B6148,
		0x0000000A,
	}
};

 cEventStack::cEventStack() {
	 m_pData = new int32[EVENT_STACK_SZ];
	 m_nIndex = 0;
	 m_nSize = EVENT_STACK_SZ;
 }

 cEventStack::~cEventStack() {
	 if (m_pData) {
		 delete[] m_pData;
#ifdef FIX_BUGS
		 m_pData = nil;
#endif
	 }
 }

void cEventStack::push(int32 id) {
	if (m_nIndex + 1 >= m_nSize) {
		debug("WARNING!! Maximum size of cEventStack exceeded. This is serious, tell Ross or Jon\n");
		return;
	}
	m_pData[m_nIndex++] = id;
}

int32 cEventStack::pop() {
	assert(!isEmpty());
	int32 id = m_pData[m_nIndex - 1];
	if (!isEmpty()) --m_nIndex;
	return id;
}

bool cEventStack::isEmpty() {
	return m_nIndex == 0;
}

void cEventStack::reset() {
	m_nIndex = 0;
}

void cEventStack::clear() {
	m_nIndex = 0;
	if (m_pData) {
		delete[] m_pData;
#ifdef FIX_BUGS
		m_pData = nil;
#endif
	}
}


#ifndef GTA_LIBERTY
cGameZoneInfo::cGameZoneInfo(int32 level) // 0, 1
{
	aActivityZonesRects = nil;
	countActivityZones = 0;
	InitialiseActivityZones(level);
}
cGameZoneInfo::~cGameZoneInfo()
{
	delete[] aActivityZonesRects;
#ifdef FIX_BUGS
	aActivityZonesRects = nil;
#endif
	countActivityZones = 0;
}

bool cGameZoneInfo::IsPointInActivityZone(CVector2D pos)
{
	CRectLeeds* zones = aActivityZonesRects;
#ifdef FIX_BUGS
	if (countActivityZones > 0 && zones != nil)
#else
	if (countActivityZones > 0)
#endif
	{
		for (int32 i = 0; i < countActivityZones; ++i, ++zones)
		{
			if (pos.x >= zones->minX && pos.x <= zones->maxX)
			{
				if (pos.y >= zones->minY && pos.y <= zones->maxY)
					return true;
			}
		}
	}
	return false;
}

void cGameZoneInfo::DrawActivityZonesOnRadar(void)
{
	if (!aActivityZonesRects || countActivityZones <= 0)
		return;

	for (int32 i = 0; i < countActivityZones; ++i) {
		CRectLeeds& z = aActivityZonesRects[i];

		// corners in world coord (ccw)
		CVector v[2]; // 0 min, 1 max
		v[0].x = z.minX; v[0].y = z.minY; v[0].z = 0.0f; // min
		v[1].x = z.maxX; v[1].y = z.maxY; v[1].z = 0.0f; // max

		// sentinel adjustments
		for (int32 t = 0; t < 2; ++t) {
			if (v[t].x == -2000.0f) v[t].x = 3000.0f;
			if (v[t].x == 1300.0f)  v[t].x = -3000.0f;
			if (v[t].y == 1800.0f)  v[t].y = 3000.0f;
			if (v[t].y == -1800.0f) v[t].y = -3000.0f;
		}

		TheRadar->RenderZonePolygonOnRadar(v[0], v[1]);
	}
}

void cGameZoneInfo::InitialiseActivityZones(int32 level) // https://prnt.sc/qRQN5tDrAW3x
{
	if (level == 1) { // mainland
		countActivityZones = 4;
		aActivityZonesRects = new CRectLeeds[countActivityZones];

		// { minX,    maxX,     minY,     maxY }
		aActivityZonesRects[0] = { -2000.0f,   120.0f,   771.0f,  1800.0f };
		aActivityZonesRects[1] = { -2000.0f,  -350.0f,     0.0f,  aActivityZonesRects[0].minY + 0.5f };   // 771.5f
		aActivityZonesRects[2] = { -2000.0f,  -150.0f,  -806.0f,  aActivityZonesRects[1].minY + 0.5f };   // 0.5f
		aActivityZonesRects[3] = { -2000.0f,  -430.0f, -1800.0f,  aActivityZonesRects[2].minY + 0.5f };   // -805.5f
	}
	else if(level == 0) { // beach
		countActivityZones = 4;
		aActivityZonesRects = new CRectLeeds[countActivityZones];

		// { minX,    maxX,     minY,     maxY }
		aActivityZonesRects[0] = { -157.0f,  1300.0f,   771.0f,  1800.0f };
		aActivityZonesRects[1] = { -350.0f,  1300.0f,     0.0f,  aActivityZonesRects[0].minY + 0.5f };   // 771.5f
		aActivityZonesRects[2] = { -715.0f,  1300.0f,  -806.0f,  aActivityZonesRects[1].minY + 0.5f };   // 0.5f
		aActivityZonesRects[3] = { -500.0f,  1300.0f, -1800.0f,  aActivityZonesRects[2].minY + 0.5f };   // -805.5f
	}
	else
		assert(0);
}
#endif





cMultiGame::cMultiGame() {
	m_bIsRemovingPeer = false;
	m_GameType = eGameType::DEATHMATCH;
#ifdef GTA_LIBERTY
	m_GameLocation = eGameLocation::IND_ZON;
#else
	m_GameLocation = eGameLocation::VICE_POINT_ZON;
#endif
	m_nScoreLimit = 10;
	m_nTimeLimit = 0;
	m_nAmbientCarBank = 0;
	m_nAmbientPedBank = 0;
	m_nScoreCTFLimit = 3;
	m_nCashTarget = 5000;

	// Flags [5]
	bPowerUpOn = true;
	eTDMStyle = eTDMStyle::FFA;
	bRacePowerUpOn = true;
	bBit_8 = false;
	bRaceRevr = false;
	bBit_20 = false;
	bIsVipTeamTeam2 = false; // miami
	bBit_80 = false;

	m_nScenarioOrRaceTrackID = 0;
	m_nRaceCarID = MI_LANDSTAL;
	ePlayIntroCutscene = static_cast<uint8>(eGameCutscenePlayback::PLAY_ONCE);
	bCutscenePlayed = false;
	m_Team1GangID = static_cast<uint8>(eGameTeam::TEAM_A);
	m_Team2GangID = static_cast<uint8>(eGameTeam::TEAM_B);
	m_nTankModelID = MI_RHINO;
#ifndef GTA_LIBERTY
	m_nHunterModelID = MI_HUNTER;
#endif
	m_nLagValue = 0;
	m_nUpdateSendTime = (uint16)-1; // !!! recheck
	m_nWaitUnk = 30;
	m_nWaitHeartBeat = 30;
	m_pNetSession = nil;
	m_EntMap = std::map<CEntity*, sElement*>();
	m_ZoneManager = cInterestZoneManager();
	field_84 = 0;
	m_nCurTime = 0;
	m_bHasSuspended = false;
	m_bIsNeedPrepareModels = false;
#ifndef GTA_LIBERTY
	m_bIsServerReadyToGo = false;
#endif
	m_nElementsIDs = 0;
	m_tPacketsEventsCB = std::map<int32, cPacketDispatcherBase*>();
	m_WaypointManager = sWaypoint();
	m_haloManager = sHalo();
#ifndef GTA_LIBERTY
	m_pLuaObject = nil;
#endif
	m_nAccTimeStep = 0;
	field_10C = -1;
	field_114 = 0;
	m_nMaskCutsceneSync = 0x0;
#ifndef GTA_LIBERTY
	m_bIsPlayerCreationQueued = false;
	m_nVipPeerID = 0;
#ifdef FIX_BUGS
	m_vecPlayerCreationQueuedPosition = CVector(0.0f, 0.0f, 0.0f);
#endif
#endif
	m_pEventStack = new cEventStack();

#ifdef FIX_BUGS // init in cMultiGame::Open()
	m_bIsConnected = false;
	m_nTargetPlayer = 100;
	m_nTimeMinutes = 0;
	m_nTimeSeconds = 0;
	m_bTimeHasSync = false;
	m_bUpdateGameTime = false;
	m_bIsRunning = false;
	m_nDefendingTeamID = 0;
	m_bShowingCommentary = false;
	m_bTeamEveryoneIn = false;
	m_nElapsedMs = 0;
	m_fConnWaitTime = 0.0f;
	m_fTimeStep = 0.0f;
	m_nTimeCentiSec = 0;
	m_nTimeSec = 0;
	for (int32 i = 0; i < MP_TEAM_COUNT; i++)
		m_abTeamTimerEnabled[i] = false;
#endif
}

cMultiGame::~cMultiGame() {
	debug("---------- Shutting down multigame\n");
	delete m_pEventStack;
	delete m_pNetSession;
}

void cMultiGame::UpdateReceive() {
	cAdhoc& Adhoc = cAdhoc::Instance();
	bool isConnected = IsOpen() && !Adhoc.HadError() && IsOpen() && m_bIsConnected && (m_pNetSession->m_nPeerCount >= MULTI_RECEIVE_PLAYER_COUNT); // double IsOpen()
	if (!isConnected) return;
	float fTimeStep = (CTimer::GetTimeStepNonClipped() * 60.0f) / 50.0f;
	m_fTimeStep = Max(1.0f, fTimeStep);
	uint16 nTime = m_nUpdateSendTime + (uint16)m_fTimeStep;
//#if 0 // oh no log
	base::string playerName;
	GetLocalPlayerName(playerName);
	MULTIGAME_LOG(1, "== P%i (%s) UpdateReceive %i\n", LocalPlayerID(), playerName.c_str(), nTime);
#ifdef DEBUG_MULTIGAME
	//debug("== MG FRAME STARTED %d\n", nTime);
#endif
//#endif
	m_pNetSession->UpdateReceive(nTime);
}

void cMultiGame::UpdateSend() {
	cAdhoc& Adhoc = cAdhoc::Instance();
	if (!IsOpen() || Adhoc.HadError() || !m_bIsConnected) {
#if !defined(FINAL) && !defined(MASTER)
		SetConsoleColor(0);
		debug("PIZDEC 1 cMultiGame::UpdateSend IsOpen():%d, Adhoc.HadError():%d, m_bIsConnected:%d\n", IsOpen(), Adhoc.HadError(), m_bIsConnected);
		SetConsoleColor(6);
#endif
		return;
	}
	if (m_nUpdateSendTime == m_pNetSession->m_nCurTime) {
#if !defined(FINAL) && !defined(MASTER)
		SetConsoleColor(0);
		debug("PIZDEC 2 cMultiGame::UpdateSend m_nUpdateSendTime:%d =?= m_pNetSession->m_nCurTime:%d\n", m_nUpdateSendTime, m_pNetSession->m_nCurTime);
		SetConsoleColor(6);
#endif
		return;
	}
	m_nUpdateSendTime = m_pNetSession->m_nCurTime;
	m_nAccTimeStep += (uint32)m_fTimeStep;
//#if 0 // oh no log
	base::string playerName;
	GetLocalPlayerName(playerName);
	MULTIGAME_LOG(1, "== P%i (%s) UpdateSend %i", LocalPlayerID(), playerName.c_str(), m_nUpdateSendTime);
//#endif
	sPeerState* pLocalPeer;
#ifdef GTA_LIBERTY
	pLocalPeer = m_vPlayers[LocalPlayerID()];
#else
	pLocalPeer = PeerManager.GetSelfPeer();
#endif
	if (pLocalPeer)
		pLocalPeer->UpdateElements(m_pNetSession->m_nCurTime);
#ifdef GTA_LIBERTY
	UpdateZonePeers();
#else
#ifdef DEBUG_MULTIGAME
	if(gbMultigameDoTimeoutStuff)
#endif
	UpdateZonePeersTimeouts();
	UpdateZonePeersSync();
#endif
	m_WaypointManager.Update();
	m_haloManager.Update();
	lsc_update_simsch();

	if (!gIsMultiplayerGame)
		Close();

	if (!IsOpen() || Adhoc.HadError() || !m_bIsConnected)
		return;
#ifdef GTA_LIBERTY
	int32 size = m_vPlayers.size();
#else
	cPeerManager& PeerMgr = PeerManager;
	int32 size = PeerMgr.m_vPlayers.size();
#endif
	for (int32 nPlayerID = 0; nPlayerID < size; nPlayerID++)
	{
#ifdef GTA_LIBERTY // TODO: CWorld size macro
		int32 sectorX = 32768;
		int32 sectorY = 32768;
		sPlayer* pPlayer = GetPlayer(nPlayerID);
		if (pPlayer) {
			CVector pos = pPlayer->GetPosition();
			sectorX = (int32)((pos.x + 2000.0f) / 4000.0f);
			sectorY = (int32)((pos.y + 2000.0f) / 4000.0f);

			//fPosX = WORLD_MIN_X + (float)m_nPosX * WORLD_SIZE_X;
			//fPosY = WORLD_MIN_Y + (float)m_nPosY * WORLD_SIZE_Y;
		}
		m_ZoneManager.UpdatePeer(nPlayerID, sectorX, sectorY);
#else
		sPeerState* peerState = PeerMgr.GetPeerAt(nPlayerID);
		m_ZoneManager.UpdatePeer(peerState->m_nID);
#endif
	}
#ifdef GTA_LIBERTY // TODO: CWorld size macro
	sPlayer* pLocalPlayer = GetPlayer(LocalPlayerID());
	if (pLocalPlayer) {
		CVector pos = pLocalPlayer->GetPosition();
		int32 sectorX = (int32)((pos.x + 2000.0f) / 4000.0f);
		int32 sectorY = (int32)((pos.y + 2000.0f) / 4000.0f);

		//fPosX = WORLD_MIN_X + (float)m_nPosX * WORLD_SIZE_X;
		//fPosY = WORLD_MIN_Y + (float)m_nPosY * WORLD_SIZE_Y;

		m_ZoneManager.UpdateAtPos(sectorX, sectorY);
	}
#else
	if (GetPlayer(LocalPlayerID()))
		m_ZoneManager.UpdatePlayer();
#endif
	if (!m_pNetSession->m_bSendLimitReached && m_pNetSession->m_nPeerCount > 0)
	{
		m_nWaitHeartBeat--;
#ifdef FIX_BUGS
		if (m_nWaitHeartBeat <= 0) // minor
#else
		if (m_nWaitHeartBeat == 0)
#endif
		{
			m_nWaitHeartBeat = MULTI_WAIT_HEART_BEAT;
			net::pckt_heart_beat packet{};
			packet.pckt_size = sizeof(net::pckt_heart_beat);
			packet.pckt_id = gtMP_PacketIDs.heart_beat.pckt_id;
			SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
}
	}
	m_pNetSession->UpdateSend();
}

void cMultiGame::SetSuspend() {
	debug("!!!!!!!!! Setting HAS SUSPENDED flag\n");
#ifdef GTA_LIBERTY
	if (field_3B && TheAdhoc.m_NextStateFuncCb != &cAdhoc::StateShutdown)
		TheAdhoc.TerminateAdhocMatching();
	TheAdhoc.bHasAckError = true;
	if (IsOpen())
		Close();
	m_bHasSuspended = true;
#else
	if (m_bHasSuspended)
		return;

	TheAdhoc.bHasError = true;
	if (IsOpen())
		Close();
#endif

}

void cMultiGame::PrepareModels() {
	m_bIsNeedPrepareModels = false;
	LoadBaseModels();
	CStreaming::RemoveUnusedModelsInLoadedList();
	CStreaming::RemoveAllUnusedModels();
	CStreaming::RemoveCurrentZonesModels();
	CStreaming::RequestModel(MI_PLAYER, STREAMFLAGS_DONT_REMOVE);
	RestoreModels();
	ShowMenu();
}

bool cMultiGame::Connect() {
	debug("Connecting Players...\n");
	if (m_bHasSuspended) {
		debug("Has Suspended, can't connect players...\n");
		Close();
		return false;
	}

	net::pckt_info packet{};
	packet.pckt_size = (sizeof(net::pckt_info) + (sizeof(tMacAddr) * 2)); // 216 // figure out why 2 mac, beta connection?
	packet.pckt_id = gtMP_PacketIDs.info.pckt_id;
	packet.nMagic = 'INFO'; // 0x494E464F
	packet.nPeerA = 1;
	packet.nPeerB = 1;
	cLobby& Lobby = cLobby::Instance();
	cAdhoc& Adhoc = cAdhoc::Instance();
	int32 nUnkPeerID = 1;
	for (int32 nPlayerID = 0; nPlayerID < MP_NUM_PEERS; nPlayerID++) {
		tAdhocPeerData& info = Lobby.m_remoteInfo.m_nPeersConnInfo[nPlayerID];
		if (info.macAddr.IsBroadcast()) {
			debug("***** Not Connecting player %d: NULL\n", nPlayerID);
			continue;
		}

		base::string playerName;
		char buffMac[20];
		info.macAddr.ToString(buffMac);
		base::string macAddr(buffMac);
		Adhoc.GetPlayerNameFromMacAddr(playerName, info.macAddr);
		debug("***** Connecting player %d: %s (%s)\n", nPlayerID, playerName.c_str(), macAddr.c_str());
		int32 nSendDest = 0;
		if (info.macAddr != Lobby.m_remoteInfo.m_HostPeerData.peerAddr.mac) {
			nSendDest = nUnkPeerID++;
		}
		if (Adhoc.IsHost()) {
			tListenAddr dest{};
			dest.mac = info.macAddr;
			dest.port = GTA_GAME_PORT;
			m_pNetSession->ConnectPeer(nSendDest, dest, 1);
		}
		else {
			packet.aPeers[nSendDest].mac = info.macAddr;
			packet.aPeers[nSendDest].port = GTA_GAME_PORT;
			packet.aPeers[nSendDest].nRandom = 0;
		}
	}
	packet.nPeerA = nUnkPeerID;
	packet.nPeerB = nUnkPeerID;
#ifndef GTA_LIBERTY
	if (Adhoc.IsHost())
		PeerManager.SetTeamPeerGroupIds();
	else
#else
	if (!Adhoc.IsHost())
#endif
	{
		m_pNetSession->ClientConnect(packet);
#ifdef GTA_LIBERTY
		int32 size = Max((int32)m_pNetSession->m_vPeers.size(), LocalPlayerID() + 1);
		m_vPlayers.resize(size);
		sPeerState* pData = m_vPlayers[0];
		m_vPlayers[LocalPlayerID()] = pData;
		m_vPlayers[0] = nil;
		if (m_vPlayers[LocalPlayerID()]) {
			m_vPlayers[LocalPlayerID()]->m_nID = LocalPlayerID();
		}
#endif
		sPeerState* selfPeer = nil;
#ifndef GTA_LIBERTY
		selfPeer = PeerManager.GetSelfPeer();
#else
		selfPeer = m_vPlayers[LocalPlayerID()];
#endif
		assert(selfPeer);
		for (auto& elemPair : selfPeer->m_vElements) {
			sElement* elem = elemPair.second;
			if (elem) elem->SetOwner(LocalPlayerID());
		}
	}
#ifndef GTA_LIBERTY
	// No additional resize or init needed, handled in ConnectPeer ?
#else
	int32 size = Max((int32)m_pNetSession->m_vPeers.size(), LocalPlayerID() + 1);
	m_vPlayers.resize(size);
	for (int32 nPeerID = 0; nPeerID < (int32)m_vPlayers.size(); nPeerID++) {
		sPeerState* pPlayer = m_vPlayers.at(nPeerID);
		if (pPlayer != nil) continue;
		pPlayer = new sPeerState(nPeerID);
		m_vPlayers.at(nPeerID) = pPlayer;
	}
#endif
	m_ZoneManager.GetZoneByPeer(LocalPlayerID()); // create new
	debug("Joined as player ID %i\n", LocalPlayerID());
	net::pckt_heart_beat beat{};
	beat.pckt_size = sizeof(net::pckt_heart_beat);
	beat.pckt_id = gtMP_PacketIDs.heart_beat.pckt_id;
	m_pNetSession->SendMessagePriority(beat, BROADCAST_PEER_GROUPID);
	return true;
}

bool cMultiGame::PerformInitialConnection() {
#if !defined(FINAL) && !defined(MASTER)
	debug("PerformInitialConnection m_fConnWaitTime %f Now %f\n", m_fConnWaitTime, CTimer::GetTimeStepNonClipped()); // custom
#endif
	cAdhoc& Adhoc = cAdhoc::Instance();
	m_bIsConnected = true;
	if (Adhoc.IsHost())
	{
		tListenAddr dest{};
		dest.mac.InitMacAddr(); // in ctor
		dest.port = GTA_GAME_PORT;
		m_pNetSession->PerformInitialConnection(dest); // in vanilla lcs vcs no dispatcher handler pckt_info
	}
	m_pNetSession->UpdateReceive(0); // cNetSession::UpdateReceive -> cNetSession::UpdateReceivePvt -> cNetSession::HandlePacketRecv -> pPeerState->bCheckSender = true
	m_pNetSession->UpdateSend(); // UpdateReceive connect peer with recv data for cMultiGame::OnGameStateChange

#ifdef GTA_LIBERTY
	//for (int32 peerID = 0; true; peerID++) {
	//	int32 size = Max((int32)m_pNetSession->m_vPeers.size(), LocalPlayerID() + 1);
	//	if (peerID >= size) break;

	//	if (!m_pNetSession->IsPeerConnected(peerID)) {
	//		if (--m_nWaitHeartBeat == 0) {
	//			m_nWaitHeartBeat = MULTI_WAIT_HEART_BEAT_2;
	//			debug("waiting for player %i\n", peerID);
	//		}
	//		m_bIsConnected = false;
	//		break;
	//	}
	//}

	int32 size = Max((int32)m_pNetSession->m_vPeers.size(), LocalPlayerID() + 1);
	for (int32 peerID = 0; peerID < size; peerID++) {
		if (!m_pNetSession->IsPeerConnected(peerID)) {
			if (--m_nWaitHeartBeat == 0) {
				m_nWaitHeartBeat = MULTI_WAIT_HEART_BEAT_2;
				debug("waiting for player %i\n", peerID);
			}
			m_bIsConnected = false;
			break;
		}
	}
#else
	cPeerManager& PeerMgr = PeerManager;
	for (int32 i = 0; i < PeerMgr.m_vPlayers.size(); ++i) {
		sPeerState* peer = PeerMgr.GetPeerAt(i);
		if (!PeerMgr.IsPeerConnected(peer->m_nID)) {
			if (--m_nWaitHeartBeat == 0) {
				m_nWaitHeartBeat = MULTI_WAIT_HEART_BEAT_2;
#if !defined(FINAL) && !defined(MASTER)
				debug("waiting connection for player %i m_fConnWaitTime %f Now %f\n",
					peer->m_nID, m_fConnWaitTime, CTimer::GetTimeStepNonClipped()); // custom
#else
				debug("waiting for player %i\n", peer->m_nID);
#endif
			}
			m_bIsConnected = false;
			break;
		}
	}
#endif

#if 0 // no fake force connected, wait all players
	if (m_bIsConnected) {
#if 0 // oh no log :(
		base::string playerName;
		Adhoc.GetPlayerNameFromMacAddr(playerName, Adhoc.GetPlayerMacAddress());
		debug("RECV LOG FOR PLAYER %i (%s)\n", LocalPlayerID(), playerName.c_str());
		debug("SEND LOG FOR PLAYER %i (%s)\n", LocalPlayerID(), playerName.c_str());
#endif
	}
	else {
		// we shouldn't be here, this allows the game with no conn
//#ifdef GTA_PC
//		TODO();
//		m_fConnWaitTime += Min(1.8f, CTimer::GetTimeStepNonClipped()); // tmp hotfix from ms_fTimeStep, mean load big step
//#else
		m_fConnWaitTime += CTimer::GetTimeStepNonClipped();
//#endif
		if (m_fConnWaitTime > MULTI_CONNECT_WAIT_TIME) {
			// fak, start mg without friend
#if !defined(FINAL) && !defined(MASTER)
			SetConsoleColor(0); // --pizdec colour warning
#endif
			assert(false && "this is bad, to out, we have no peer connection");
			debug("**** Initial Connection Has Timed Out!!!!\n");
#if !defined(FINAL) && !defined(MASTER)
			SetConsoleColor(6);
#endif
			m_bIsConnected = true;
		}
#ifdef GTA_PC
		std::this_thread::sleep_for(std::chrono::milliseconds(500)); // psp mp load slow hle eat?
#endif
	}
#endif

#if !defined(FINAL) && !defined(MASTER)
	debug("**** QUIT FROM cMultiGame::PerformInitialConnection WITH m_bIsConnected %s\n", m_bIsConnected ? "OK" : "FALSE!!(not connected some peer padla)");
#endif
	return m_bIsConnected;
}

#ifdef GTA_LIBERTY
int32 cMultiGame::GetNumberOfPeersConnected() {
	int32 nCount = 0;
	for (int32 i = 1; i < m_vPlayers.size(); i++) {
		if (m_pNetSession->IsPeerConnected(i))
			++nCount;
	}
	return nCount;
}
#endif

uint16 cMultiGame::AdjustSendTime(uint16 time, uint16 peer) {
	uint16 nCurTime = m_pNetSession->m_nCurTime;
	uint16 nPeerTime = (peer != LocalPlayerID()) ? m_pNetSession->m_vPeers.at(peer)->nTime_D : nCurTime;
	return nCurTime + (time - nPeerTime);
}

// lcs only, probably debug thing got stripped
void cMultiGame::OnAckRecv(uint16 nPeerID) {
	//debug("cMultiGame::OnAckRecv(%d)\n", nPeerID); // guessed
}

#ifndef GTA_LIBERTY
void cMultiGame::UpdatePlayerLatency(uint8 nPeerID, int32 latencyMs) {
	PeerManager.UpdatePeerLatency(nPeerID, latencyMs);
}
#endif

bool cMultiGame::IsLocalPlayer(int32 id) {
	return m_pNetSession->IsLocalPlayer(id);
}

bool cMultiGame::IsSameGroup(int32 a, int32 b) {
	return m_pNetSession->IsSameGroup(a, b);
}

sPlayer* cMultiGame::GetPlayer(int32 nPeerID) {
	if (nPeerID < 0) nPeerID = LocalPlayerID();
	return (sPlayer*)GetEntityForHandle(nPeerID, eElementID::MG_ELEMENT_PLAYER_ID);
}

sPed* cMultiGame::GetPlayerPed(int32 nPeerID) {
	if (nPeerID < 0) nPeerID = LocalPlayerID();
	return (sPed*)GetEntityForHandle(nPeerID, eElementID::MG_ELEMENT_PLAYER_PED_ID);
}

bool cMultiGame::IsPlayerConnected(int32 nPeerID) {
#ifdef GTA_LIBERTY
	if (nPeerID < (int32)m_vPlayers.size() && m_vPlayers.at(nPeerID) != nil) return true;
	return false;
#else
	return PeerManager.IsPeerConnected(nPeerID);
#endif
}

void cMultiGame::SetPlayerName(const char* name) {
#ifdef GTA_LIBERTY
	m_vPlayers.at(LocalPlayerID())->m_sName = name;
#else
	sPeerState* peer = PeerManager.GetPeerById(LocalPlayerID());
	assert(peer != nil);
	peer->m_sName = base::string(name);
#endif
}

const char* cMultiGame::GetPlayerName(int32 id) {
#ifdef GTA_LIBERTY
	if (m_vPlayers.at(id) != nil && m_pNetSession->IsPeerConnected(id))
#else

	sPeerState* peer = PeerManager.GetPeerById(id);
	if (peer != nil && PeerManager.IsPeerConnected(id))
#endif
	{
#ifdef GTA_LIBERTY
		sPeerState* peer = m_vPlayers.at(id);
		const char* pStr = peer->m_sName.c_str();
		if (!pStr) {
			base::string sPlayerName;
			cAdhoc::Instance().GetPlayerNameFromMacAddr(sPlayerName, m_pNetSession->m_vPeers.at(id)->m_addr.mac);
			peer->m_sName = sPlayerName;
			return peer->m_sName.c_str();
		}
		return pStr;
#else
#ifdef FIX_BUGS
		if (peer->m_sName.empty())
			cAdhoc::Instance().GetPlayerNameFromMacAddr(peer->m_sName, peer->m_Addr.mac);
		return peer->m_sName.c_str();
#else
		base::string nickname = peer->PeerName();
		if (nickname.length() != 0) {
			// no way...
			// https://media1.tenor.com/m/fzsMyzVDamoAAAAd/danny-devito-no.gif
			char nicknameBuff[20]; // no SCE_NET_ADHOCCTL_NICKNAME_LEN?
			wchar nicknameBuffW[12];
			sprintf(nicknameBuff, "%s", nickname.c_str());
			AsciiToUnicode(nicknameBuff, nicknameBuffW);
			return UnicodeToAscii(nicknameBuffW);
		}
#endif
#endif
	}
	return UnicodeToAscii(TheText.Get("NO_NAME"));
}

void cMultiGame::GetLocalPlayerName(base::string& outName) {
	cAdhoc& Adhoc = cAdhoc::Instance();
	Adhoc.GetPlayerNameFromMacAddr(outName, Adhoc.GetPlayerMacAddress());
}

const char* cMultiGame::GetGangName(uint16 id) {
	if(id == static_cast<uint16>(eGameTeam::TEAM_A))
		return UnicodeToAscii(TheText.Get(gMPGangDefs[GetTeam1GangID()].name));
	else if (id == static_cast<uint16>(eGameTeam::TEAM_B))
		return UnicodeToAscii(TheText.Get(gMPGangDefs[GetTeam2GangID()].name));
	assert(false && "unknown gang id");
	return nil;
}

const char* cMultiGame::GetGangNameForEntity(uint16 id) {
	sElement* pElem = GetEntityForHandle(id, eElementID::MG_ELEMENT_PLAYER_PED_ID); // Game.GetPlayerPed()
	int32 nTeamID = id;
	if (pElem && pElem->GetType() == eElementType::ELEMENT_TYPE_PED)
		nTeamID = ((sPed*)pElem)->GetTeamID();
	return GetGangName(nTeamID);
}

int32 cMultiGame::GetPlayerTeamID(int32 id) {
	sElement* pElem = GetEntityForHandle(id, eElementID::MG_ELEMENT_PLAYER_PED_ID); // Game.GetPlayerPed()
	if (pElem && pElem->GetType() == eElementType::ELEMENT_TYPE_PED)
		return ((sPed*)pElem)->GetTeamID();
	return -1;
}

bool cMultiGame::IsAnyTeamEmpty() {
	if (!m_bTeamEveryoneIn)
		return false;

	int32 nTeamACount = 0;
	int32 nTeamBCount = 0;

	for (int32 i = 0; i < PeerManager.m_vPlayers.size(); ++i) {
		sPeerState* peer = PeerManager.m_vPlayers[i];
		if (!peer || !peer->IsConnected()) continue;

		int32 teamId = GetPlayerTeamID(peer->m_nID);
		if (teamId == static_cast<int32>(eGameTeam::TEAM_A)) ++nTeamACount;
		else if (teamId == static_cast<int32>(eGameTeam::TEAM_B)) ++nTeamBCount;
	}

	if (nTeamACount > 0 && nTeamBCount > 0)
		return false;

	debug("TEAM 0 has %d players\n", nTeamACount);
	debug("TEAM 1 has %d players\n", nTeamBCount);
	return true;
}

bool cMultiGame::GetCutsceneSkipEnabled() {
#ifdef GTA_LIBERTY
	// TODO: implement cutscene skip
	if (static_cast<eGameCutscenePlayback>(m_playIntroCutscene) == eGameCutscenePlayback::ALWAYS_PLAY)
		return false;
	if (static_cast<eGameCutscenePlayback>(m_playIntroCutscene) == eGameCutscenePlayback::DONT_PLAY)
		return true;
	// todo return (m_playIntroCutscene & 0x80) != 0;
	return false;
#else
	return true;
#endif
}

void cMultiGame::SetGameType(eGameType type)
{
	m_GameType = type;
}

eGameType cMultiGame::GetGameType()
{
	return m_GameType;
}

void cMultiGame::SetGameLocation(eGameLocation location)
{
	m_GameLocation = Min(location, (eGameLocation)(((int)eGameLocation::NUM_MP_GAME_LOCATION) - 1));
}

eGameLocation cMultiGame::GetGameLocation()
{
	return m_GameLocation;
}

void cMultiGame::SetScoreLimit(int32 limit) {
	m_nScoreLimit = limit;
}

int32 cMultiGame::GetScoreLimit() {
	return m_nScoreLimit;
}

void cMultiGame::SetTimeLimit(int32 limit) {
	m_nTimeLimit = limit;
}

int32 cMultiGame::GetTimeLimit() {
	return m_nTimeLimit;
}

void cMultiGame::SetCTFScoreLimit(int32 limit) {
	m_nScoreCTFLimit = limit;
}

int32 cMultiGame::GetCTFScoreLimit() {
	return m_nScoreCTFLimit;
}

void cMultiGame::SetCashTarget(int32 target) {
	m_nCashTarget = target;
}

int32 cMultiGame::GetCashTarget() {
	return m_nCashTarget;
}

void cMultiGame::SetTeam1GangID(uint8 id) {
	m_Team1GangID = id;
}

uint8 cMultiGame::GetTeam1GangID() {
	return m_Team1GangID;
}

void cMultiGame::SetTeam2GangID(uint8 id) {
	m_Team2GangID = id;
}

uint8 cMultiGame::GetTeam2GangID() {
	return m_Team2GangID;
}

static CRGBA gMP_colors[12] = {
	CRGBA(174, 0, 0, 255),
	CRGBA(77, 155, 210, 255),
	CRGBA(75, 151, 75, 255),
	CRGBA(217, 174, 87, 255),
	CRGBA(252, 116, 186, 255),
	CRGBA(151, 82, 197, 255),
	CRGBA(240, 158, 147, 255),
	CRGBA(255, 153, 51, 255),
	CRGBA(255, 227, 79, 255),
	CRGBA(174, 0, 0, 255),
	CRGBA(255, 255, 255, 255),
	CRGBA(153, 153, 153, 255)
};

static CRGBA gMP_ColorBlack(0, 0, 0, 0);

CRGBA* cMultiGame::GetColor(int32 id) { // This was also used in SCORESHEET.LUA Colour(6) Colour(10)
#ifdef FIX_BUGS
	assert(id < ARRAY_SIZE(gMP_colors));
	return &gMP_colors[id % ARRAY_SIZE(gMP_colors)];
#else
	return &gMP_colors[id % 16];
#endif
}

CRGBA* cMultiGame::GetBlipColor(int32 id) {
	return &gMP_colors[id];
}

CRGBA* cMultiGame::GetTeamColor(int32 id) {
	sElement* pElem = GetEntityForHandle(id, eElementID::MG_ELEMENT_PLAYER_PED_ID); // cMultiGame::GetPlayerPed()
	if (pElem != nil && pElem->GetType() == eElementType::ELEMENT_TYPE_PED) {
		uint8 nTeamID = ((sPed*)pElem)->GetTeamID();
		return GetColor(nTeamID);
	}
	return &gMP_ColorBlack;
}

CRGBA* cMultiGame::GetPlayerColor(int32 id) {
	int32 nColorID = 0;
#ifdef FIX_BUGS
	if (m_GameType != eGameType::HITPARADE) nColorID = id % ARRAY_SIZE(gMP_colors);
#else
	if (m_GameType != eGameType::HITPARADE) nColorID = id % 16;
#endif
	else nColorID = (id != m_nTargetPlayer) ? 2 : 0;
	return &gMP_colors[nColorID];
}

void cMultiGame::SyncPlayerDead(CEntity* pEntity) {
	MARKFUNCTION(0x0, 0x08ADC8D4);

	sPlayer* pKiller = pEntity != nil && pEntity->IsMultiplayer() ? ((cPhysicalMG*)pEntity)->GetElement().player : nil;
	uint8 nKillerID = (pKiller != nil && pKiller->GetID() <= eElementID::MG_ELEMENT_PLAYER_PED_ID) ? pKiller->GetOwner() : LocalPlayerID();

	net::pckt_player_kill packet{};
	packet.pckt_size = sizeof(net::pckt_player_kill);
	packet.pckt_id = gtMP_PacketIDs.player_kill.pckt_id;
	packet.assassin = nKillerID;
	OnPlayerKill(packet, LocalPlayerID(), m_pNetSession->m_nCurTime, true);
	SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
	//SyncPlayerDead(nKillerID); // or can simple call overload
}

void cMultiGame::SyncPlayerDead(uint8 id) {
	MARKFUNCTION(0x0, 0x08ADC994);

	net::pckt_player_kill packet{};
	packet.pckt_size = sizeof(net::pckt_player_kill);
	packet.pckt_id = gtMP_PacketIDs.player_kill.pckt_id;
	packet.assassin = id;
	OnPlayerKill(packet, LocalPlayerID(), m_pNetSession->m_nCurTime, true);
	SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
}

void cMultiGame::SendMessage(const net::pckt_base& packet, int destID) {
	if (!m_bIsConnected) return;
	m_pNetSession->SendMessage(packet, destID);
}

void cMultiGame::SendMessagePriority(const net::pckt_base& packet, int destID) {
	if (!m_bIsConnected) return;
	m_pNetSession->SendMessagePriority(packet, destID);
}

sElement* cMultiGame::GetEntityForHandle(int32 nPeerID, int16 id) {
#ifdef GTA_LIBERTY
	if (owner < (int32)m_vPlayers.size()) {
		return m_vPlayers.at(owner)->FindElement(nPeerID);
	}
#else
	if (!PeerManager.IsPeerConnected(nPeerID))
		return nil;

	sPeerState* peer = PeerManager.GetPeerById(nPeerID);
	auto it = peer->m_vElements.find(static_cast<uint16>(id));
	if (it != peer->m_vElements.end()) {
		return it->second;
	}

	// same logic
	//sPeerState* peer = PeerManager.GetPeerById(nPeerID);
	//for (auto it = peer->m_vElements.begin(); it != peer->m_vElements.end(); ++it) {
	//	if (it->first == static_cast<uint16>(id)) {
	//		return it->second;
	//	}
	//}
#endif
	return nil;
}

//sElement* cMultiGame::FindElement(CEntity* ent) {
//	for (int32 i = 0; i < m_vEntList.size(); i++) {
//		if (m_vEntList.at(i).first == ent) {
//			return m_vEntList.at(i).second;
//		}
//	}
//	return nil;
//}

#ifdef GTA_LIBERTY
//todo recheck
sElement* cMultiGame::FindElement(int16 nPeerID, int32 nOwner, int16 nElemID) {
	if (nPeerID >= m_vPlayers.size())
		return nil;

	if (!m_vPlayers[nPeerID])
		return nil;

	for (auto it = m_vPlayers[nPeerID]->m_vElements.begin(); it != m_vPlayers[nPeerID]->m_vElements.end(); ++it) {
		if (it->second->m_nPrevOwnerID == nOwner && it->second->m_nPrevID == nElemID)
			return it->second;
	}
	return nil;
}
#else
sElement* cMultiGame::FindElement(int16 nPeerID, int32 nOwner, int16 nElemID) {
	if (!PeerManager.IsPeerConnected(nPeerID))
		return nil;

	sPeerState* peer = PeerManager.GetPeerById(nPeerID);
	for (auto it = peer->m_vElements.begin(); it != peer->m_vElements.end(); ++it) {
		if (it->second->m_nPrevOwnerID == nOwner && it->second->m_nPrevID == nElemID)
			return it->second;
	}
	return nil;
}
#endif

// TODO: compat GTA_LIBERTY
sVehicle* cMultiGame::FindVehicle(int16 nPeer, int16 nDriverID) {
	if (!PeerManager.IsPeerConnected(nPeer))
		return nil;

	sPeerState* peer = PeerManager.GetPeerById(nPeer);

	for (auto it = peer->m_vElements.begin(); it != peer->m_vElements.end(); ++it) {
		sElement* elem = it->second;
		if (!elem)
			continue;

		eElementType type = elem->GetType();
		if (type == eElementType::ELEMENT_TYPE_AUTOMOBILE ||
			type == eElementType::ELEMENT_TYPE_BIKE ||
#ifdef FIX_BUGS
			type == eElementType::ELEMENT_TYPE_HELI ||
			type == eElementType::ELEMENT_TYPE_BOAT ||
			type == eElementType::ELEMENT_TYPE_PLANE ||
			type == eElementType::ELEMENT_TYPE_BMX ||
			type == eElementType::ELEMENT_TYPE_QUADBIKE
#else
			type == eElementType::ELEMENT_TYPE_HELI
#endif
			)
		{
			assert(elem->HasCapability(sVehicle::Capability()));
			if (elem->GetSync().vehicle->m_nDriverID == nDriverID)
				return (sVehicle*)elem;
		}
	}

	return nil;
}

//template<typename T>
//T cMultiGame::GetElementFromEntity(CEntity* entity)
//{
//	for (std::vector<std::pair<CEntity*, sElement*>>::iterator it = m_vEntList.begin(); it != m_vEntList.end(); it++) {
//		if (it->first == entity) return (T)it->second;
//	}
//	return nil;
//}

#ifdef GTA_LIBERTY
void cMultiGame::TransferEntity(sElement* elem) { SendTransferEntityMsg(elem, LocalPlayerID()); }
#else
bool cMultiGame::TransferEntity(sElement* elem) { return SendTransferEntityMsg(elem, LocalPlayerID()); }
#endif

void cMultiGame::SendDemandEntityMsg(sElement* elem)
{
	if (elem->GetOwner() != LocalPlayerID()) {
		debug("((((((((( Sending DemandEntity Message %i, %i to %i\n", elem->GetOwner(), elem->GetID(), LocalPlayerID());
		CEntity* pEnt = elem->GetEntity();
		if (pEnt) {
			debug("Entity is type %d Model Id %d\n", (int32)elem->GetType(), pEnt->GetModelIndex());
		}
		net::pckt_transfer_entity packet;
		packet.pckt_size = sizeof(net::pckt_transfer_entity);
		packet.pckt_id = gtMP_PacketIDs.transfer_entity.pckt_id;
		packet.src = elem->GetOwner();
		packet.dest = LocalPlayerID();
		packet.elem = elem->GetID();
		SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
		return;
	}
#ifdef GTA_LIBERTY
	if (!elem->WasTransfered()) {
		elem->TransferEntity(LocalPlayerID());
	}
#else
	sElementPhysical* pElementPhysical = nil;
	if (elem && elem->HasCapability(sElementPhysical::Capability()))
		pElementPhysical = (sElementPhysical*)elem;

	if ((!pElementPhysical || pElementPhysical->HasAcksFromAllPeers()) && !elem->WasTransfered()) {
		elem->TransferEntity(LocalPlayerID());
	}
#endif
}

#ifdef GTA_LIBERTY
void cMultiGame::SendTransferEntityMsg(sElement* elem, int16 id)
{
	if (elem->GetOwner() != LocalPlayerID() && elem->IsTransferable()) {
		debug("Sending Transfer Entity Message %i, %i to %i\n", elem->GetOwner(), elem->GetID(), id);
		CEntity* pEnt = elem->GetEntity();
		if (pEnt != nil)
			debug("Entity is type %d Model Id %d\n", (int32)elem->GetType(), pEnt->GetModelIndex());
		///SendTransferPacket(elem, id);

		net::pckt_transfer_entity packet;
		packet.pckt_size = sizeof(net::pckt_transfer_entity);
		packet.pckt_id = gtMP_PacketIDs.transfer_entity.pckt_id;
		packet.src = elem->GetOwner();
		packet.dest = id;
		packet.elem = elem->GetID();
		SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
	}
	else if (!elem->WasTransfered() && elem->IsTransferable())
		elem->TransferEntity(id);
}
#else
bool cMultiGame::SendTransferEntityMsg(sElement* elem, int16 id)
{
	sElementPhysical* pElementPhysical = nil;
	if (elem && elem->HasCapability(sElementPhysical::Capability()))
		pElementPhysical = (sElementPhysical*)elem;

	if (pElementPhysical && pElementPhysical->GetOwner() == LocalPlayerID() && !pElementPhysical->HasAcksFromAllPeers()) {
		return false;
	}

	if (elem->GetOwner() != LocalPlayerID()) {
		if (elem->IsTransferable()) {
			debug("Sending Transfer Entity Message %i, %i to %i\n", elem->GetOwner(), elem->GetID(), id);
			CEntity* pEnt = elem->GetEntity();
			if (pEnt) {
				debug("Entity is type %d Model Id %d\n", (int32)elem->GetType(), pEnt->GetModelIndex());
			}
			net::pckt_transfer_entity packet;
			packet.pckt_size = sizeof(net::pckt_transfer_entity);
			packet.pckt_id = gtMP_PacketIDs.transfer_entity.pckt_id;
			packet.src = elem->GetOwner();
			packet.dest = id;
			packet.elem = elem->GetID();
			SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
		}
	}
	else if (!elem->WasTransfered() && elem->IsTransferable()) {
		elem->TransferEntity(id);
	}
	return true;
}
#endif

void cMultiGame::Nop1() {
	;
}

bool cMultiGame::IsElementExists(int16 nPeerID, sElement* elem) {
	sPeerState* peer = PeerManager.GetPeerById(nPeerID);
	for (auto it = peer->m_vElements.begin(); it != peer->m_vElements.end(); ++it) {
		if (it->second == elem) {
			return true;
		}
	}
	return false;
}

void cMultiGame::RegisterPacket(int32 packet_id, cPacketDispatcherBase* pDispatcher) {
	assert(packet_id > -1 && pDispatcher);

	// Increment the ref count of the new dispatcher
	if (pDispatcher)
		++pDispatcher->m_nRefCount;

	auto it = m_tPacketsEventsCB.find(packet_id);
	cPacketDispatcherBase* pOldDispatcher = nil;
	if (it != m_tPacketsEventsCB.end())
		pOldDispatcher = it->second;

	//assert(!m_tPacketsEventsCB[packet_id]); // 1st reg pastebug recheck
	m_tPacketsEventsCB[packet_id] = pDispatcher;

	// Dispose of the old dispatcher if it existed
	if (pOldDispatcher && --pOldDispatcher->m_nRefCount == 0)
		delete pOldDispatcher;
}

#if !defined(FINAL) && !defined(MASTER)
void cMultiGame::PrintRegisteredPackets()
{
#if 0
	SetConsoleColor(1);
	for (const auto& pair : m_tPacketsEventsCB) {
		if(pair.second->GetID() == 0)
			printf("ID: %-4d %-30s CB: 0x%-12p CTX: 0x%-12p\n", pair.first, gtMP_PacketIDs.GetPacketName(i),
				((cPacketDispatcher*)pair.second)->m_pFunctionCB, nil);
		else if (pair.second->GetID() == 1)
			printf("ID: %-4d %-30s CB: 0x%-12p CTX: 0x%-12p\n", pair.first, gtMP_PacketIDs.GetPacketName(i),
				((cPacketDispatcherMultiGame*)pair.second)->m_pFunctionCB, ((cPacketDispatcherMultiGame*)pair.second)->m_pCTX);
		else
			printf("ID: %-4d %-30s CB: 0x%-12p CTX: 0x%-12p\n", pair.first, gtMP_PacketIDs.GetPacketName(i),
				((cPacketDispatcherWaypoint*)pair.second)->m_pFunctionCB, ((cPacketDispatcherWaypoint*)pair.second)->m_pCTX);
	}
	SetConsoleColor(6);
#else
	SetConsoleColor(1);
	debug("Locked Club imba\n");
	debug("=========== Size compat test ===========\n");
	gtMP_PacketIDs.TestSizes();

	debug("\n\n=========== Dispatcher test ===========\n");
	for (int32 i = 0; i < net::packet_id_list_t::snPacketCount; ++i) {
		cPacketDispatcherBase* dispatcher = m_tPacketsEventsCB[i];
		SetConsoleColor(dispatcher != nil);
		if (!dispatcher)
			printf("ID: %-4d %-30s CB: 0x%-12p CTX: 0x%-12p\n", i, gtMP_PacketIDs.GetPacketName(i), nil, nil);
		else if (dispatcher->GetID() == 0)
			printf("ID: %-4d %-30s CB: 0x%-12p CTX: 0x%-12p\n", i, gtMP_PacketIDs.GetPacketName(i),
				((cPacketDispatcher*)dispatcher)->m_pFunctionCB, nil);
		else if (dispatcher->GetID() == 1)
			printf("ID: %-4d %-30s CB: 0x%-12p CTX: 0x%-12p\n", i, gtMP_PacketIDs.GetPacketName(i),
				((cPacketDispatcherMultiGame*)dispatcher)->m_pFunctionCB, ((cPacketDispatcherMultiGame*)dispatcher)->m_pCTX);
		else
			printf("ID: %-4d %-30s CB: 0x%-12p CTX: 0x%-12p\n", i, gtMP_PacketIDs.GetPacketName(i),
				((cPacketDispatcherWaypoint*)dispatcher)->m_pFunctionCB, ((cPacketDispatcherWaypoint*)dispatcher)->m_pCTX);
	}
	SetConsoleColor(6);
#endif
}
#endif

void cMultiGame::LoadScene() {
#ifdef GTA_LIBERTY
	CBridge::ForceBridgeState(STATE_BRIDGE_LOCKED);
	// TODO: implement cWorldStream calls
	CTheScripts::UndoBuildingSwaps();
	// TODO: calls to CTheScripts
	CTheScripts::SwapNearestBuildingModel(674.0f, -929.0f, 113.0f, 20.0f, 3807, 3814);
	CTheScripts::SwapNearestBuildingModel(733.0f, -929.0f, 38.0f, 20.0f, 3804, 3815);
	CTheScripts::SwapNearestBuildingModel(677.0f, -929.0f, 45.0f, 100.0f, 3820, 3861);
	CTheScripts::SwapNearestBuildingModel(674.0f, -929.0f, 42.0f, 50.0f, 3810, 3816);
	CTheScripts::SwapNearestBuildingModel(674.0f, -929.0f, 43.0f, 50.0f, 3809, 3817);
	CTheScripts::SwapNearestBuildingModel(674.0f, -929.0f, 44.0f, 50.0f, 3811, 3818);
	CTheScripts::SwapNearestBuildingModel(674.0f, -929.0f, 42.0f, 50.0f, 3812, 3819);
	CTheScripts::SwapNearestBuildingModel(558.0f, -929.0f, 113.0f, 20.0f, 3832, 3829);
	CTheScripts::SwapNearestBuildingModel(498.0f, -929.0f, 38.0f, 30.0f, 3831, 3828);
	CTheScripts::SwapNearestBuildingModel(555.0f, -929.0f, 45.0f, 100.0f, 3835, 3864);
	CTheScripts::SwapNearestBuildingModel(558.0f, -929.0f, 42.0f, 100.0f, 3833, 3822);
	CTheScripts::SwapNearestBuildingModel(557.0f, -929.0f, 43.0f, 100.0f, 3834, 3824);
	CTheScripts::SwapNearestBuildingModel(557.0f, -929.0f, 44.0f, 100.0f, 3825, 3823);
	CTheScripts::SwapNearestBuildingModel(557.0f, -929.0f, 42.0f, 100.0f, 3826, 3827);
	int32 nObjRef;
	if (GetGameLocation() == eGameLocation::COM_ZON) {
		nObjRef = CreateObjectAtPos(480, 178.041f, -1594.106f, 15.35f);
		AvoidObjectCleanup(nObjRef);
		UpdateObjectHeading(nObjRef, 180.0f);
		nObjRef = CreateObjectAtPos(480, 187.041f, -1594.106f, 15.30f);
		AvoidObjectCleanup(nObjRef);
		UpdateObjectHeading(nObjRef, 180.0f);
		ThePaths.SwitchPedRoadsOffInArea(175.0f, 186.0f, -1630.0f, -1568.0f, -25.0f, 20.0f, true);
		nObjRef = CreateObjectAtPos(480, 224.041f, -109.106f, 10.60f);
		AvoidObjectCleanup(nObjRef);
		UpdateObjectHeading(nObjRef, 180.0f);
		nObjRef = CreateObjectAtPos(480, 233.041f, -109.106f, 10.60f);
		AvoidObjectCleanup(nObjRef);
		UpdateObjectHeading(nObjRef, 180.0f);
		ThePaths.SwitchPedRoadsOffInArea(207.0f, 541.79f, -165.0f, -103.1f, -14.0f, 12.0f, true);
	}
	else if (GetGameLocation() == eGameLocation::SUB_ZON) {
		nObjRef = CreateObjectAtPos(480, -672.0f, -760.00f, 8.30f);
		AvoidObjectCleanup(nObjRef);
		UpdateObjectHeading(nObjRef, 90.0f);
		nObjRef = CreateObjectAtPos(480, -672.0f, -768.90f, 8.30f);
		AvoidObjectCleanup(nObjRef);
		UpdateObjectHeading(nObjRef, 90.0f);
		ThePaths.SwitchPedRoadsOffInArea(-775.79f, -622.0f, -827.0f, -750.09f, -18.0f, 25.0f, true);
		nObjRef = CreateObjectAtPos(3841, -332.5f, 92.49f, -19.0f);
		AvoidObjectCleanup(nObjRef);
		UpdateObjectHeading(nObjRef, 225.0f);
	}
	else {
		nObjRef = CreateObjectAtPos(480, 988.96f, -471.77f, 5.20f);
		AvoidObjectCleanup(nObjRef);
		UpdateObjectHeading(nObjRef, 90.0f);
		ThePaths.SwitchPedRoadsOffInArea(-541.79f, -207.0f, -165.0f, -103.1f, -14.0f, 12.0f, true);
	}
#else
	// VCS: empty
#endif
}

uint32 cMultiGame::GetPlayersCount() {
	return (uint32)PeerManager.m_vPlayers.size();
}

int32 cMultiGame::GetNumberOfVehicles() {
	return sVehicle::ms_nNumberOfSyncedVehicles;
}

void cMultiGame::UpdateTime() {
	cAdhoc& Adhoc = cAdhoc::Instance();
	if (!Adhoc.IsHost() || !m_bUpdateGameTime)
		return;

	for (int32 teamID = 0; teamID < MP_TEAM_COUNT; teamID++) {
		if (!m_abTeamTimerEnabled[teamID])
			continue;

		m_anTeamTimer[teamID] -= CTimer::GetTimeStepInMilliseconds();
	}
	uint32 nCurTime = CTimer::GetTimeInMilliseconds();
	if ((nCurTime - m_nTimeCentiSec) > 101) {
		m_nTimeCentiSec = nCurTime;
		m_nElapsedMs += 101;
	}
	if ((nCurTime - m_nTimeSec) > 1000) {
		m_nTimeSec = nCurTime;

		if (m_nTimeSeconds > 0) {
			--m_nTimeSeconds;
		}
		else if (m_nTimeMinutes > 0) {
			--m_nTimeMinutes;
			m_nTimeSeconds = 59;
		}
		else {
			// minutes==0 && seconds==0
		}
	}
	net::pckt_game_time packet{};
	packet.pckt_size = sizeof(net::pckt_game_time);
	packet.pckt_id = gtMP_PacketIDs.game_time.pckt_id;
	packet.time.min = m_nTimeMinutes;
	packet.time.sec = m_nTimeSeconds;
	packet.elapsedMs = m_nElapsedMs;
	packet.nTeamATime = m_anTeamTimer[static_cast<int32>(eGameTeam::TEAM_A)];
	packet.nTeamBTime = m_anTeamTimer[static_cast<int32>(eGameTeam::TEAM_B)];
	SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
	m_bTimeHasSync = true;
}

int32 cMultiGame::GetTimeMinutes() {
	return m_nTimeMinutes;
}

int32 cMultiGame::GetTimeSeconds() {
	return m_nTimeSeconds;
}

void cMultiGame::SetTimeMinutes(int32 value) {
	m_nTimeMinutes = value;
}

void cMultiGame::SetTimeSeconds(int32 value) {
	m_nTimeSeconds = value;
}

void cMultiGame::SetGameElapsedMs(uint32 value) {
	m_nElapsedMs = value;
}

uint32 cMultiGame::GetGameElapsedMs() {
	return m_nElapsedMs;
}

bool cMultiGame::IsGameTimeUp() {
	return GetTimeLimit() > 0 && GetTimeMinutes() == 0 && GetTimeSeconds() == 0 && m_bTimeHasSync;
}

void cMultiGame::SetTargetPlayer(int32 player, bool send) {
	m_nTargetPlayer = player;
	if (send) {
		net::pckt_target_player packet;
		packet.pckt_size = sizeof(net::pckt_target_player);
		packet.pckt_id = gtMP_PacketIDs.target_player.pckt_id; // 20
		packet.player = player;
		SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
	}
}

int32 cMultiGame::GetTargetPlayer() {
	return m_nTargetPlayer;
}

int32 cMultiGame::GetSpawnPointFromPlayer(int32 id) {
	if (id > MP_NUM_PEERS) return 1;
	return m_aPlayerID[id];
}

void cMultiGame::SetDefendingTeamID(uint8 id) {
	m_nDefendingTeamID = id;
}

uint8 cMultiGame::GetDefendingTeamID() {
	return m_nDefendingTeamID;
}

bool cMultiGame::HasPlayerJoinedGame(uint8 id) {
#ifdef GTA_LIBERTY
	for (std::vector<sPeerState*>::iterator it = m_vPlayers.begin(); it != m_vPlayers.end(); it++) {
		sPeerState* pInfo = *it;
		if (pInfo->m_nID == id) {
			return pInfo->m_bIsSpawned;
		}
	}
#else
	return PeerManager.GetPeerById(id)->m_bIsSpawned;
#endif
	return false;
}

void cMultiGame::SetPlayerSpawned(int32 id) {
#ifdef GTA_LIBERTY
	for (std::vector<sPeerState*>::iterator it = m_vPlayers.begin(); it != m_vPlayers.end(); it++) {
		sPeerState* pInfo = *it;
		if (pInfo->m_nID == id) {
			pInfo->m_bIsSpawned = true;
			break;
		}
	}
#else
	PeerManager.GetPeerById(id)->m_bIsSpawned = true;
#endif
}

void cMultiGame::SetCutscenePlaying(int32 id) {
	m_nMaskCutsceneSync |= BIT(id);
}

int32 cMultiGame::GetSyncedCarCount() {
	if (!IsOpen())
		return 0;

	sPeerState* selfPeer = PeerManager.GetSelfPeer();
	if (!selfPeer)
		return 0;

	int32 count = 0;

	for (auto it = selfPeer->m_vElements.begin(); it != selfPeer->m_vElements.end(); ++it) {
		sElement* elem = it->second;
		if (!elem || elem->GetOwner() != LocalPlayerID())
			continue;

		eElementType type = elem->GetType();
		if (type == eElementType::ELEMENT_TYPE_AUTOMOBILE ||
			type == eElementType::ELEMENT_TYPE_BIKE ||
#ifdef FIX_BUGS
			type == eElementType::ELEMENT_TYPE_HELI ||
			type == eElementType::ELEMENT_TYPE_BOAT ||
			type == eElementType::ELEMENT_TYPE_PLANE ||
			type == eElementType::ELEMENT_TYPE_BMX ||
			type == eElementType::ELEMENT_TYPE_QUADBIKE
#else
			type == eElementType::ELEMENT_TYPE_HELI
#endif
			)
		{
			assert(elem->GetEntity()->IsVehicle());
			if (elem->GetEntity() && elem->GetEntity() && ((CVehicle*)elem->GetEntity())->VehicleCreatedBy != MISSION_VEHICLE)
				++count;
		}
	}

	return count;
}

void cMultiGame::RemovePlayerFromGame(int32 id) {
	cAdhoc& Adhoc = cAdhoc::Instance();
	if (LocalPlayerID() == id) {
		m_bIsConnected = false;
		Adhoc.bHasError = true;
		return;
	}
	debug("************ cMultiGame::RemovePlayerFromGame\n");

	for (int32 i = 0; i < PeerManager.m_vPlayers.size(); ++i) {
		sPeerState* peer = PeerManager.m_vPlayers[i];
		if (peer) {
			const char* name = GetPlayerName(peer->m_nID);
			debug("RemovePlayerFromGame: %s\n", name);
		}
	}
	PeerManager.GetLastPeer();

	for (int32 nPeerID = 0; nPeerID < MP_NUM_PEERS; nPeerID++) {
		tAdhocPeerData& peer = cLobby::Instance().m_remoteInfo.m_nPeersConnInfo[nPeerID];
		if (!peer.macAddr.IsBroadcast() && nPeerID == id) {
			char scriptBuf[256];
			base::string sPlayerName;
			Adhoc.GetPlayerName(sPlayerName, peer);
#ifdef GTA_LIBERTY
			sprintf(scriptBuf, "main.game:Commentate('^S^%s ^T^MPLEFT')", sPlayerName.c_str());
#else
			sprintf(scriptBuf, "coreLib:Commentate('^S^%s ^T^MPLEFT')", sPlayerName.c_str());
#endif
			cLWrapper& pWrapper = cLWrapper::Instance();
			pWrapper.ExecString(scriptBuf);
			break;
		}
	}

	m_bIsRemovingPeer = true;
	m_ZoneManager.RemovePeerFromAllZones(id);
	m_pNetSession->DisconnectPeer(id);
#ifdef GTA_LIBERTY
	for (std::vector<sPeerState*>::iterator it = m_vPlayers.begin(); it != m_vPlayers.end(); it++) {
		sPeerState* pInfo = *it;
		if (pInfo->m_nID == id) {
			delete pInfo;
			*it = nil;
			break;
		}
	}
#endif
	m_bIsRemovingPeer = false;
}

#ifdef GTA_LIBERTY
int32 cMultiGame::CreateObjectAtPos(int16 index, float posX, float posY, float posZ) {
	if (index < 0)
		index = CTheScripts::UsedObjectArray[-index].index;
	CObject* pObj = new CObject(index, false);
	pObj->ObjectCreatedBy = MISSION_OBJECT;
	if (posZ <= MAP_Z_LOW_LIMIT)
		posZ = CWorld::FindGroundZForCoord(posX, posY);
	pObj->SetPosition(posX, posY, posZ);
	pObj->SetOrientation(0, 0, 0);
	pObj->GetMatrix().UpdateRW();
	pObj->UpdateRwFrame();
	CTheScripts::ClearSpaceForMissionEntity(CVector(posX, posY, posZ), pObj);
	CWorld::Add(pObj);
	return CPools::GetObjectRef(pObj);
}

void cMultiGame::AvoidObjectCleanup(int32 ref) {
	CObject* pObj = CPools::GetObject(ref);
	CTheScripts::MissionCleanUp.RemoveEntityFromList(ref, CLEANUP_OBJECT);
}

void cMultiGame::UpdateObjectHeading(int32 ref, float rot) {
	CObject* pObj = CPools::GetObject(ref);
	CWorld::Remove(pObj);
	pObj->SetHeading(DEGTORAD(rot));
	pObj->GetMatrix().UpdateRW();
	pObj->UpdateRwFrame();
	CWorld::Add(pObj);
}
#else
bool cMultiGame::CreateLuaObject(int32 index, CVector pos)
{
	if (m_pLuaObject)
		return false;

	//m_pLuaObject = new CObject(index, true); // beta?
	m_pLuaObject = new CObject(MI_PICKUP_BRIEFCASE, true);
	if (!m_pLuaObject)
		return false;

	m_pLuaObject->SetPosition(pos);
	m_pLuaObject->SetOrientation(0, 0, 0);
	m_pLuaObject->GetMatrix().UpdateRW();
	m_pLuaObject->UpdateRwFrame();
	MULTIGAME_UNIMPLEMENTED(); // TODO FLAGS
	m_pLuaObject->ObjectCreatedBy = eObjectCreatedBy::CONTROLLED_SUB_OBJECT;
	MULTIGAME_UNIMPLEMENTED(); // TODO FLAGS
	m_pLuaObject->ObjectCreatedBy = eObjectCreatedBy::MISSION_OBJECT; // why
	CWorld::Add(m_pLuaObject);
	return true;
}

bool cMultiGame::DestroyLuaObject()
{
	if (!m_pLuaObject)
		return false;

	CWorld::Remove(m_pLuaObject/*, 1*/);
	delete m_pLuaObject;
	m_pLuaObject = nil;
	return true;
}

void cMultiGame::RequestPlayerCreation(CVector pos) {
	m_bIsPlayerCreationQueued = true;
	m_vecPlayerCreationQueuedPosition = pos;
}

void cMultiGame::ShowMenu() {
	cAdhoc& Adhoc = cAdhoc::Instance();
	//FrontEndMenuManager->cmenumanager_set_sub_882E99C(0);
	gIsMultiplayerGame = false;
	if (!FrontEndMenuManager->GetIsMenuActive())
	{
		FrontEndMenuManager->Initialise2();
		FrontEndMenuManager->SetHasJoinedNetwork(false);

		TODO();
		TODO();
		TODO();
		//FrontEndMenuManager->fe_mp_sub_882F210(true); // Multiplayer_t MP_ERROR_PAGE "MP_P1_PAGE"
		 // into in fe_mp_sub_882F210 temp stack
		{
			FrontEndMenuManager->LoadAllTextures();
			DMAudio.PlayFrontEndSound(SOUND_FRONTEND_MENU_STARTING, 0);
			DMAudio.ChangeMusicMode(MUSICMODE_FRONTEND);
			DMAudio.Service();
			FrontEndMenuManager->m_bMenuActive = true;
			FrontEndMenuManager->SetSinglePlayerMode();
		}

		FrontEndMenuManager->SetHasJoinedNetwork(true);

		TODO();
		TODO();
		TODO();
		//FrontEndMenuManager->sub_882F6E8("MP_P1_PAGE");

		FrontEndMenuManager->SetMultigameMode();
	}

	if (!Adhoc.HadError())
		TheLobby.ReturnAfterGame();
}
#endif


#ifdef GTA_LIBERTY
/* TODO#2 */
void cMultiGame::UpdateZonePeers() {
	sElementPhysical::ms_nNumberOfSyncedPhysicals = 0;
	if (!m_bIsConnected)
		return;
	cInterestZone* pZone = m_ZoneManager.GetZoneByPeer(MP_HOST_INDEX);
	cAdhoc& Adhoc = cAdhoc::Instance();
	uint32 nTimeSinceLastUpdate = CTimer::GetTimeInMilliseconds() - m_nCurTime;
	bool hasLost = false;
	for (size_t peerID = 0; nTimeSinceLastUpdate >= MULTI_TIME_OUT_3 && peerID < pZone->m_vPeers.size(); peerID++) // lcs 10000, vcs 25000
	{
		tZonePeer& peer = pZone->m_vPeers.at(peerID);
		bool isConnected = m_pNetSession->IsPeerConnected(peer.id);
		bool isAckEmpty = pZone->PeerLastAckEmpty(peer.id);
		int isTimeOut = m_pNetSession->m_nCurTime - pZone->PeerLastAck(peer.id) > MULTI_TIME_OUT;
		if (isConnected || isAckEmpty || isTimeOut) {
			if (!isConnected) debug("!mpNetPeer->IsPeerConnected(peerId)\n");
			if (isAckEmpty) debug("pZone->PeerLastAckEmpty(peerId)\n");
			if (isTimeOut) debug("time - pZone->PeerLastAck(peerId) > MULTI_TIME_OUT\n");
			if (peerID == 0) {
				hasLost = true;
				break;
			}
			if (Adhoc.m_bHasStartedMatching) {
				debug("Peer %d has timed out, removing them and telling everyone else to do the same\n", peerID);
				RemovePlayerFromGame(peerID);
				net::pckt_kick_player packet{};
				packet.pckt_size = sizeof(net::pckt_kick_player);
				packet.pckt_id = gtMP_PacketIDs.kick_player.pckt_id;
				packet.peer_id = (uint8)peerID;
				SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
			}
		}
	}

	if (hasLost) {
		debug("I haven't heard from the server, i'm leaving the game. Last Ack == %d\n", pZone->PeerLastAck(0));
		m_bIsConnected = false;
		Adhoc.bHasAckError = true;
		return;
	}

#ifdef GTA_LIBERTY
	for (int32 peerID = 0; peerID < m_vPlayers.size(); peerID++)
#else
	for (int32 peerID = 0; peerID < PeerManager.m_vPlayers.size(); peerID++)
#endif
	{
#ifdef GTA_LIBERTY
		sPeerState* pPlayer = m_vPlayers.at(peerID);
#else
		sPeerState* pPlayer = PeerManager.m_vPlayers.at(peerID);
#endif
		if (pPlayer == nil || m_pNetSession->IsPeerConnected(peerID)) continue;
		uint16 nCurTime = 0;
		if (peerID == LocalPlayerID())
			nCurTime = m_pNetSession->m_nCurTime;
		else
		{
			TODO();
			TODO();
			TODO();
			//int32 computedTime = pPlayer->m_nCurTime + m_fTimeStep;
			//uint16_t prevTimeB = pPlayer->m_nTimeB;
			//uint16_t remotePeerField = m_pNetSession->GetPeerField(peerState->m_nID);

			//if (computedTime < static_cast<int>(prevTimeB - 4))
			//	computedTime = prevTimeB - 4;
			//if (computedTime < static_cast<int>(prevTimeB - 2))
			//	computedTime++; 
			//if (computedTime > static_cast<int>(remotePeerField - 2))
			//	computedTime--; 
			//if (computedTime > static_cast<int>(remotePeerField))
			//	computedTime = remotePeerField;

			//peerState->nTimeB = remotePeerField;
			//peerState->nCurTime = static_cast<uint16_t>(computedTime);
			//nCurTime = static_cast<uint16_t>(computedTime);

			nCurTime = 0; // tmp
		}
	}
}
#else
void cMultiGame::UpdateZonePeersTimeouts() {
	cAdhoc& Adhoc = cAdhoc::Instance();
	cPeerManager& PeerMgr = PeerManager;
	uint16 nCurTime = m_pNetSession->m_nCurTime;
	cInterestZone* pZone = m_ZoneManager.GetZoneByPeer(MP_HOST_INDEX);

	for (int32 nIndex = 0; nIndex < PeerMgr.m_vPlayers.size(); ++nIndex)
	{
		sPeerState* pPeer = PeerMgr.GetPeerAt(nIndex);
		uint32 nPeerID = pPeer->m_nID;

		if (nPeerID == LocalPlayerID())
			continue;

		if ((CTimer::GetTimeInMilliseconds() - m_nCurTime) < MULTI_TIME_OUT_3) // 25000, nTimeSinceLastUpdate
			continue;

		bool isConnected = PeerMgr.IsPeerConnected(nPeerID);
		bool isAckEmpty = pZone->PeerLastAckEmpty(nPeerID);
		uint16 nAck = pPeer->PeerLastAck();
		bool isTimeOut = (nCurTime - nAck) > MULTI_TIME_OUT_2;
		if (!isConnected || isAckEmpty || isTimeOut) {
			// vcs has double IsPeerConnected PeerLastAckEmpty PeerLastAck call for log
			if (!isConnected)
				debug("!mpNetPeer->IsPeerConnected(peerId)\n");
			if (isAckEmpty)
				debug("pZone->PeerLastAckEmpty(peerId)\n");
			if (isTimeOut)
				debug("time - pZone->PeerLastAck(peerId) > MULTI_TIME_OUT\n");
			if (nPeerID == 0) {
				debug("I haven't heard from the server, i'm leaving the game. Last Ack == %d\n", pZone->PeerLastAck(0));
				m_bIsConnected = false;
				Adhoc.SetHasError();
				return;
			}

			if (Adhoc.IsHost())
			{
				debug("Peer %d has timed out, removing them and telling everyone else to do the same\n", nPeerID);
				RemovePlayerFromGame(nPeerID);
				net::pckt_kick_player packet{};
				packet.pckt_size = sizeof(net::pckt_kick_player);
				packet.pckt_id = gtMP_PacketIDs.kick_player.pckt_id;
				packet.peer_id = (uint8)nPeerID;
				SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
			}
		}
	}
}

void cMultiGame::UpdateZonePeersSync() {
	sElementPhysical::ms_nNumberOfSyncedPhysicals = 0;
	if (!m_bIsConnected)
		return;

	cPeerManager& PeerMgr = PeerManager;
	for (int32 nIndex = 0; nIndex < PeerMgr.m_vPlayers.size(); ++nIndex) {
		sPeerState* pPeer = PeerMgr.GetPeerAt(nIndex);
		if (!pPeer->IsConnected())
			continue;

		uint16 nCurTime;
		if (pPeer->m_nID == LocalPlayerID()) {
			nCurTime = m_pNetSession->m_nCurTime;
		}
		else
		{
			nCurTime = pPeer->m_nTimeA + m_fTimeStep;
			uint16 nOldTimeB = pPeer->m_nTimeB;
			uint16 nRemoteTime = m_pNetSession->GetPeerTime(pPeer->m_nID);
			pPeer->m_nTimeB = nRemoteTime;
			if (nCurTime < (nOldTimeB - 4))
				nCurTime = nOldTimeB - 4;
			if (nCurTime < (nOldTimeB - 2))
				++nCurTime;
			if ((nRemoteTime - 2) < nCurTime)
				--nCurTime;
			if (nRemoteTime < nCurTime)
				nCurTime = nRemoteTime;
			pPeer->m_nTimeA = nCurTime;
		}

		for (auto& elemPair : pPeer->m_vElements) {
			sElement* pElement = elemPair.second;
			if (pElement) {
				uint16 nLastSyncTime = pElement->m_vSync.front().m_nTime;
				if (nCurTime >= nLastSyncTime)
				{
					if (pElement->GetOwner() != LocalPlayerID())
					{
#ifdef FIX_BUGS
						assert(pElement->m_pZone->GetZonePeer(pPeer->m_nID)); // recheck 2+ players zone size, despite this there is still a bug
						uint32 nBasis = pElement->m_pZone->GetZonePeer(pPeer->m_nID)->nBasis;
#else
						//assert(pElement->m_pZone->m_vPeers.size() > pPeer->m_nID); // 1 > 0
						uint32 nBasis = pElement->m_pZone->m_vPeers[pPeer->m_nID].nBasis; // !! WARN vanilla bug here, read invalid data after vector
#endif
						uint16 nRemoteTime = m_pNetSession->GetPeerTime(pPeer->m_nID);
						pElement->DisposeAttachedDelta(nRemoteTime, nBasis);
					}

					pElement->ApplyClientSync(nCurTime);
				}
			}
		}
	}
}
#endif

void cMultiGame::RegisterEntity(sElement* pElem) {
	uint16 nOwner = pElem->GetOwner();
	if (nOwner == LocalPlayerID()) 
		pElem->SetID(GetNextElementID());
#ifdef GTA_LIBERTY
	m_vPlayers.at(nOwner)->InsertElement(pElem);
#else
	cPeerManager& PeerMgr = PeerManager;
	PeerMgr.GetPeerById(nOwner)->InsertElement(pElem);
#endif
}

void cMultiGame::RemoveElement(sElement* elem) {
	uint16 nOwner = elem->GetOwner();
#ifdef GTA_LIBERTY
	sPeerState* pPeer = m_vPlayers.at(nOwner);
#else
	cPeerManager& PeerMgr = PeerManager;
	if (!PeerMgr.IsPeerConnected(nOwner)) return;
	sPeerState* pPeer = PeerMgr.GetPeerById(nOwner);
#endif
	if (pPeer == nil) return;
	pPeer->RemoveElement(elem); // remove in peer->m_vElements
	RemoveEntity(elem);
}

void cMultiGame::AttachEntity(sElement* elem, CEntity* entity) {
	RemoveEntity(elem);
	if (entity == nil) return;
	m_EntMap[entity] = elem;
}

uint16 cMultiGame::GetNextElementID() {
	uint16 id = m_nElementsIDs;
#ifdef GTA_LIBERTY
	TODO();
#else
	cPeerManager& PeerMgr = PeerManager;
	sPeerState* peer = PeerMgr.GetSelfPeer();
#endif
	while (peer->m_vElements.find(id) != peer->m_vElements.end()) {
		++id;
	}
	++m_nElementsIDs;
	return id;
}

void cMultiGame::SendTransferPacket(sElement* elem, int8 to) {
#ifndef GTA_LIBERTY
	sElementPhysical* pElementPhysical = nil;
	if (elem && elem->HasCapability(sElementPhysical::Capability()))
		pElementPhysical = (sElementPhysical*)elem;

	if (pElementPhysical && !pElementPhysical->HasAcksFromAllPeers())
		return;
#endif

	net::pckt_transfer_entity packet{};
	packet.pckt_size = sizeof(net::pckt_transfer_entity);
	packet.pckt_id = gtMP_PacketIDs.transfer_entity.pckt_id;
	packet.src = elem->GetOwner();
	packet.dest = to;
	packet.elem = elem->GetID();
	SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
}

void cMultiGame::DiscardModels() {
#if defined(FIX_BUGS) && !defined(GTA_LIBERTY)
	CStreaming::SetModelIsDeletable(m_nHunterModelID);
#endif
	CStreaming::SetModelIsDeletable(m_nRaceCarID);
	CStreaming::SetModelIsDeletable(MI_STRETCH);
	CStreaming::SetModelIsDeletable(MI_IDAHO);
	CStreaming::SetModelIsDeletable(MI_STINGER);
#ifdef GTA_LIBERTY
	CStreaming::SetModelIsDeletable(MI_ESPRIT);
#endif
	CStreaming::SetModelIsDeletable(MI_PONY);
	CStreaming::SetModelIsDeletable(MI_STRETCH);
	CStreaming::SetModelIsDeletable(MI_RHINO);
	CStreaming::SetModelIsDeletable(MI_MPLAYCRATE); // LCS:4010  VCS: 7381
	CStreaming::SetModelIsDeletable(MI_MPLAYCRATE_DOORA); // LCS:4011  VCS: 7383
	CStreaming::SetModelIsDeletable(MI_MPLAYCRATE_DOORB); // LCS:4012  VCS: 7382
}

void cMultiGame::RestoreModels() {
#ifdef GTA_LIBERTY
	CStreaming::RequestModel(MI_POLICE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_COP, STREAMFLAGS_DONT_REMOVE);
#else
	CStreaming::LoadInitialPeds();
	CStreaming::LoadAllRequestedModels(false);
	CStreaming::LoadInitialWeapons();
	CStreaming::LoadAllRequestedModels(false);
	CStreaming::LoadInitialVehicles();
#endif
	CStreaming::LoadAllRequestedModels(false);
}

void cMultiGame::LoadGameModels() {
#ifdef GTA_LIBERTY
	switch (GetGameType())
	{
		case eGameType::DEFENDTHEBASE:
			CStreaming::RequestModel(MI_STRETCH, MP_LOAD_FLAGS);
			if (GetGameLocation() == eGameLocation::IND_ZON) {
				CStreaming::RequestModel(MI_IDAHO, MP_LOAD_FLAGS);
				CStreaming::RequestModel(MI_STINGER, MP_LOAD_FLAGS);
			}
			else {
				if (GetGameLocation() == eGameLocation::COM_ZON)
					CStreaming::RequestModel(MI_STINGER, MP_LOAD_FLAGS);
				else if (GetGameLocation() == eGameLocation::SUB_ZON)
					CStreaming::RequestModel(MI_PONY, MP_LOAD_FLAGS);
				CStreaming::RequestModel(MI_ESPRIT, MP_LOAD_FLAGS);
			}
			break;
		case eGameType::CTF:
			CStreaming::RequestModel(MI_STRETCH, MP_LOAD_FLAGS);
			break;
		case eGameType::TANK:
			CStreaming::RequestModel(MI_RHINO, MP_LOAD_FLAGS);
			break;
		case eGameType::SIXTYSECONDS:
			CStreaming::RequestModel(MI_CRATE_SJL, MP_LOAD_FLAGS); // 279 4010
			CStreaming::RequestModel(MI_DOOR1_SJL, MP_LOAD_FLAGS); // 280 4011
			CStreaming::RequestModel(MI_DOOR2_SJL, MP_LOAD_FLAGS); // 281 4012
			CStreaming::RequestModel(MI_ESPERANT, MP_LOAD_FLAGS);
			CStreaming::RequestModel(MI_STINGER, MP_LOAD_FLAGS);
			CStreaming::RequestModel(MI_BANSHEE, MP_LOAD_FLAGS);
			break;
		case eGameType::HITPARADE:
			CStreaming::RequestModel(m_nRaceCarID, MP_LOAD_FLAGS);
			break;
	}
	CStreaming::LoadPedbanksForMultiplayer();
	CStreaming::LoadZoneVehicle(CVector(0.0f, 0.0f, 0.0f));
	loadWeapons();
	if (GetGameLocation() == eGameLocation::IND_ZON)
		CColStore::LoadCollision(CVector(1044.0f, -822.0f, 15.0f), eLevelName::LEVEL_INDUSTRIAL);
	else if (GetGameLocation() == eGameLocation::COM_ZON)
		CColStore::LoadCollision(CVector(296.0f, -1180.0f, 25.5f), eLevelName::LEVEL_COMMERCIAL);
	else if (GetGameLocation() == eGameLocation::SUB_ZON)
		CColStore::LoadCollision(CVector(-500.0f, 81.0f, 3.0f), eLevelName::LEVEL_SUBURBAN);
	CStreaming::LoadAllRequestedModels(false);
#else
	for (int32 i = 0; i < TheEmpire->GetEmpireCount(); i++)
	{
		CEmpireBuildingInfo* pInfo = TheEmpire->GetEmpireInfo(i);
		pInfo->SetBusinessType(EMPIRE_BUSINESS_PROTECTION);
		pInfo->SetScaleLevel(EMPIRE_SCALE_LEVEL_1);
		pInfo->SetBuildingState(eEmpireState::EMPIRE_NONE);
#ifdef FIX_BUGS
		pInfo->SetCondition(0.0f); // lol, after mg, map tab bug
#endif
	}
	TheEmpire->Process();
	CStreaming::LoadAllRequestedModels(false);
	TheEmpire->Process();

	CStreaming::RequestModel(MI_PLAYER, STREAMFLAGS_DONT_REMOVE);
	CStreaming::LoadInitialPeds();
	CStreaming::LoadAllRequestedModels(false);
	CStreaming::LoadInitialWeapons();
	CStreaming::LoadAllRequestedModels(false);
	CStreaming::LoadInitialVehicles();
	CStreaming::LoadAllRequestedModels(false);

	switch (GetGameType())
	{
		case eGameType::MULTIRACE:
			LoadMultiraceGameTypeModels(); // inlined
			break;
		case eGameType::DEFENDTHEBASE:
			LoadDefendTheBaseGameTypeModels();
			break;
		case eGameType::CTF:
			LoadCtfGameTypeModels();
			break;
		case eGameType::TANK:
			CStreaming::RequestModel(MI_RHINO, (MP_LOAD_FLAGS | STREAMFLAGS_SCRIPTOWNED));
			break;
		case eGameType::SIXTYSECONDS:
			CStreaming::RequestModel(MI_ESPERANT, (MP_LOAD_FLAGS | STREAMFLAGS_SCRIPTOWNED));
			CStreaming::RequestModel(MI_STINGER, (MP_LOAD_FLAGS | STREAMFLAGS_SCRIPTOWNED));
			CStreaming::RequestModel(MI_BANSHEE, (MP_LOAD_FLAGS | STREAMFLAGS_SCRIPTOWNED));
			break;
		case eGameType::HITPARADE:
			CStreaming::RequestModel(MI_HUNTER, (MP_LOAD_FLAGS | STREAMFLAGS_SCRIPTOWNED));
			break;
		case eGameType::FLAGBALL:
		case eGameType::VIP:
			CStreaming::RequestModel(MI_PICKUP_BRIEFCASE, (MP_LOAD_FLAGS | STREAMFLAGS_SCRIPTOWNED));
			break;
	}

	CStreaming::LoadPedbanksForMultiplayer();
	CStreaming::LoadCarbanksForMultiplayer(); // inlined
	loadWeapons();
	CStreaming::LoadAllRequestedModels(false);
#endif
}

/* TODO: partially done, missing some subroutines

- cSmallHeap thing can be delayed indefinitely, unless someone wants to port this to the PSP
-
*/
void cMultiGame::LoadBaseModels() {
#ifndef GTA_LIBERTY
	TheEmpire->RemoveAllActualEmpiresBuildings();
	CStreaming::PrintStreamingBufferStateToConsole();
#endif
	CStreaming::FlushRequestList();
	while (CStreaming::RemoveLoadedVehicle());
	while (CStreaming::RemoveLoadedZoneModel());

	for (int32 nIndex = 0; nIndex < NUM_DEFAULT_MODELS; nIndex++)
	{
		if (!CStreaming::IsObjectInCdImage(nIndex)) continue;
		CStreamingInfo* pInfo = &CStreaming::ms_aInfoForModel[nIndex];
		if (pInfo->m_loadState == STREAMSTATE_LOADED && (pInfo->m_flags & STREAMFLAGS_40) == 0) {
			debug("Removing Model with index %d\n", nIndex);
			if ((pInfo->m_flags & STREAMFLAGS_DONT_REMOVE) != 0)
				CStreaming::SetModelIsDeletable(nIndex);
			if ((pInfo->m_flags & STREAMFLAGS_SCRIPTOWNED) != 0)
				CStreaming::SetMissionDoesntRequireModel(nIndex);
			if ((pInfo->m_flags & STREAMFLAGS_AMBIENT_SCRIPT_OWNED) != 0)
				CStreaming::SetAmbientMissionDoesntRequireModel(nIndex);
			pInfo->m_flags = pInfo->m_flags & 0xFE; // maintain all flags, expect allow to remove
			if (nIndex < CModelInfo::GetNumModelInfos()) {
				uint8 type = CModelInfo::GetModelInfo(nIndex)->GetModelType();
				if (type == MITYPE_VEHICLE) CStreaming::RemoveLoadedVehicle();
				else if (type == MITYPE_PED) CStreaming::RemoveLoadedZoneModel();
			}
		}
	}
#ifndef GTA_LIBERTY
	CColStore::RemoveAllCollision();
	CStreaming::PrintStreamingBufferStateToConsole();
#endif
	CStreaming::SetModelIsDeletable(MI_POLICE);
	CStreaming::ms_aInfoForModel[0].m_flags = STREAMFLAGS_DONT_REMOVE;
	debug("After:\n");
	CStreaming::RemoveAllUnusedModels();
	CStreaming::RemoveUnusedModelsInLoadedList();
	CStreaming::PrintStreamingBufferStateToConsole();
	CStreaming::FlushRequestList();
	// TODO(LVCS): cSmallHeap calls
#ifdef GTA_LIBERTY
	CStreaming::RequestModel(MI_PLAYER, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_COP, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_MALE01, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_TAXI_D, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_NIGHTSTICK, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_MISSILE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_MISSILE, STREAMFLAGS_DONT_REMOVE); // huh?
	CStreaming::RequestModel(MI_CAMERA, STREAMFLAGS_DONT_REMOVE);
	CStreaming::LoadAllRequestedModels(false);
#endif
	gbMP_RenderHudExtras = false;
}

// Done
void cMultiGame::Open() {
	debug("XXXXXX cMultiGame::Open()\n");
#ifndef GTA_LIBERTY
	m_bHasSuspended = false;
	m_bIsNeedPrepareModels = false;
	m_bIsServerReadyToGo = false;
#ifdef GTA_WORLDSTREAM
	WorldStream->Reset();
#else
	CStreaming::RestoreBuildingSwaps();
#endif
	LoadBaseModels();
#endif
	debug("%s", animDebug.nAnimHashes);
	// cSmallHeap
	m_nCurTime = CTimer::GetTimeInMilliseconds();
	debug("Doing net session VOLALLOC\n");
	tListenAddr listen_addr;
	listen_addr.port = GTA_GAME_PORT;
	listen_addr.mac = cAdhoc::Instance().GetPlayerMacAddress();
	char buffMac[20];
	sceNetEtherNtostr(cAdhoc::Instance().GetPlayerMacAddress().GetBytesSCE(), buffMac);
	base::string sPlayerMac = base::string(buffMac); // useless or beta log
	for (int32 i = 0; i < sPlayerMac.length(); i++)
		if (sPlayerMac[i] == ':') sPlayerMac[i] = '.';
	//debug("Player MAC: %s\n", sPlayerMac);
#ifndef GTA_LIBERTY
	SetGameZoneInfo(new cGameZoneInfo((int32)(GetGameLocation() >= eGameLocation::DOWNTOWN_ZON ? eGameLocation::MAINLAND_LVL : eGameLocation::BEACH_LVL)));
#endif
#ifdef GTA_LIBERTY
	m_pNetSession = new cNetSession('GTA3', 0); // 0x47544133
#else
	m_pNetSession = new cNetSession('VICE', 0); // 0x56494345
#endif
#ifdef MP_USE_CUSTOM_ALLOCATOR
	m_pNetSession->SetAllocator(allocFunc, deleteFunc);
	PeerManager.SetAllocator(allocFunc, deleteFunc);
#endif
	m_pNetSession->StartPDPListen(listen_addr);
	m_pNetSession->m_pPacketDispatcher = new cPacketDispatcherMultiGame(&cMultiGame::FireMessageHandler, this);
	RegisterPacket(gtMP_PacketIDs.transfer_entity.pckt_id, new cPacketDispatcherMultiGame((PcktRecvHndMultiGame)&cMultiGame::OnTransferEntity, this)); // 17
	RegisterPacket(gtMP_PacketIDs.game_state.pckt_id, new cPacketDispatcherMultiGame((PcktRecvHndMultiGame)&cMultiGame::OnGameStateChange, this)); // 14
	RegisterPacket(gtMP_PacketIDs.player_kill.pckt_id, new cPacketDispatcher(&cMultiGame::OnPlayerKill)); // 3
	mp_regiter_packets();

	cNavArrow::ClearTarget();
	gbMP_DrawPauseScreen = false;
	gbMP_DrawPauseScreenNoBox = false;
#ifndef GTA_LIBERTY
	gbMP_HudShowHelp = false;
#endif
	gMultiplayerSuperBrakeOnPause = true;
	CCarCtrl::ToggleScriptControlsMpCarLimit(false);
	CCarCtrl::SetMultiplayerAmbientCarLimit(20);
#ifndef GTA_LIBERTY
#ifdef GTA_FERRIS_WHEEL
	CFerrisWheel::Shutdown();
#endif
	CDarkel::Init();
#endif
	mp_register_waypoint_packets(&m_WaypointManager);
#if !defined(FINAL) && !defined(MASTER)
	PrintRegisteredPackets();
#endif
	sHalo::Init();
	m_ZoneManager.GetZoneByPeer(MP_HOST_INDEX); // if not found create new zone
#ifdef GTA_LIBERTY // :/
	m_bIsConnected = false;
	m_fTimeStep = 1.0f;
#else
	m_fTimeStep = 1.0f;
	m_bIsConnected = false;
#endif
	m_fConnWaitTime = 0.0f;
	field_E8 = 0;
	m_nTimeMinutes = GetTimeLimit();
	m_nTimeSeconds = 0;
	m_nElapsedMs = 0;
	m_nTimeSec = 0;
	m_nTimeCentiSec = 0;
	m_bUpdateGameTime = false;
	for (int32 i = 0; i < MP_TEAM_COUNT; i++) // unroolled loop
		m_abTeamTimerEnabled[i] = false;
	m_bTimeHasSync = false;
	m_bShowingCommentary = false;
#ifdef GTA_LIBERTY
	LoadBaseModels();
#endif
	LoadGameModels();
#ifndef GTA_PC // in psp is disabled, todo figure out collision (+ in psp worldstream build, non pc ipl)
	CStreaming::DisableStreaming();
#else
	assert(CStreaming::ms_disableStreaming == false);
#endif
	m_nTargetPlayer = 100;
	field_10C = -1;
	field_114 = 0;
	m_bIsRunning = false;
	m_nMaskCutsceneSync = 0x0;
	CGame::currArea = eAreaName::AREA_MAIN_MAP;
	CTimeCycle::StopExtraColour(false);
	m_bTeamEveryoneIn = false;
#ifndef GTA_LIBERTY
	CWeather::ForceWeatherNow(WEATHER_EXTRASUNNY); // 4
	CSwimBoundary::DisableBoundary();
	gFireManager.ExtinguishAll();
	SetFlagBallPosition(CVector(99999.0f, 99999.0f, 99999.0f));
	m_pEventStack->reset();
	FrontEndMenuManager->Shutdown();
#endif
}

// Done
void cMultiGame::Close() {
#define DO_HEAP_STUFF() // probably process heap block cleanup
	debug("XXXXXX cMultiGame::Close()\n");
#ifndef GTA_LIBERTY
	m_bIsNeedPrepareModels = m_bHasSuspended;
	m_bIsServerReadyToGo = false;
	if (FindPlayerPed())
		FindPlayerPed()->m_bIsCenterBlipVisible = true;
#endif
	CTimer::StartUserPause(); // inlined
	TheCamera.m_WideScreenOn = false;
	DO_HEAP_STUFF();
	cLScript::Shutdown();
	DO_HEAP_STUFF();
	gb_mp_will_destroy_elem = true;
#ifdef GTA_LIBERTY
	sTextSprite::Terminate();
#else
	sSpriteBase::Terminate();
#endif
	DO_HEAP_STUFF();
	m_WaypointManager.Reset();
	DO_HEAP_STUFF();
	m_haloManager.Reset();
	DO_HEAP_STUFF();
	gb_mp_will_destroy_elem = false;
#ifndef GTA_LIBERTY
#ifdef GTA_FERRIS_WHEEL
	CFerrisWheel::Shutdown();
#endif
	CDarkel::Init();
	cNavArrow::ClearTarget();
	gFireManager.ExtinguishAll();
#endif
	m_pEventStack->reset();
	DO_HEAP_STUFF();
#ifndef GTA_LIBERTY
	if (m_pLuaObject) {
		CWorld::Remove(m_pLuaObject, eWorldRemoveType::WORLD_REMOVE_WITH_CLEANUP_VEHICLES);
		delete m_pLuaObject;
		m_pLuaObject = nil;
	}
#endif
	DO_HEAP_STUFF();
	DiscardModels();
	DO_HEAP_STUFF();
	// sce
	//dword_8B5E3C8 = 0; // VCS dword_8BB45C8 = 0;
	//dword_8B5E3C4 = 0; // VCS dword_8BB45C4 = 0;
	m_bIsRemovingPeer = true;
	DO_HEAP_STUFF();
	m_ZoneManager.Terminate();
	DO_HEAP_STUFF();
#ifndef GTA_LIBERTY
	PeerManager.Terminate();
	DO_HEAP_STUFF();
#endif
#ifndef GTA_LIBERTY
	if (IsOpen()) {
		m_pNetSession->Terminate();
		delete m_pNetSession;
		m_pNetSession = nil;
	}
	else if (m_pNetSession != nil) {
		delete m_pNetSession;
		m_pNetSession = nil;
	}
#else
	if (IsOpen()) {
		m_pNetSession->Terminate();
		for (int32 id = 0; id < m_vPlayers.size(); id++) {
			sPeerState* pPeer = m_vPlayers.at(id);
			if (pPeer == nil) continue;
			debug("Deleting player %d\n", id);
			delete pPeer;
			m_vPlayers.at(id) = nil;
		}
		delete m_pNetSession;
		m_pNetSession = nil;
	}
#endif
	DO_HEAP_STUFF();
	m_bIsRemovingPeer = false;
	m_bIsConnected = false;
	m_fConnWaitTime = 0.0f;
#ifndef FIX_BUGS
	DO_HEAP_STUFF();
#endif
	CTheScripts::UndoBuildingSwaps();
#ifdef FIX_BUGS
	DO_HEAP_STUFF();
#endif
	m_nElementsIDs = 0;
	m_nLagValue = 0;
	m_nUpdateSendTime = (uint16)-1; // recheck!!!!
	m_nWaitUnk = MULTI_WAIT_HEART_BEAT;
	m_nWaitHeartBeat = MULTI_WAIT_HEART_BEAT;
	field_84 = 0;
	m_nAccTimeStep = 0;
	m_GameType = eGameType::DEATHMATCH;
#ifdef GTA_LIBERTY
	m_GameLocation = eGameLocation::IND_ZON;
#else
	m_GameLocation = eGameLocation::VICE_POINT_ZON;
#endif
	m_nScoreLimit = 10;
	m_nTimeLimit = 0;
	m_nScoreCTFLimit = 3;
	// Flags [5]
	bPowerUpOn = true;
	eTDMStyle = static_cast<uint8>(eTDMStyle::FFA);
	bRacePowerUpOn = true;
	bBit_8 = false;
	bRaceRevr = false;
	bBit_20 = false;
	bIsVipTeamTeam2 = false; // miami
	bBit_80 = false;
	// End flags
	m_nScenarioOrRaceTrackID = 0;
	m_nRaceCarID = MI_LANDSTAL;
	m_nDefendingTeamID = static_cast<uint8>(eGameTeam::TEAM_B);
	DO_HEAP_STUFF();
	TheRadar->ClearAllBlips();
	DO_HEAP_STUFF();
	TheRadar->m_ShowMapPlayerPos = false;
	CMessages::ClearPreviousBriefArray();
	CMessages::AddToPreviousBriefArray(TheText.Get("MP_SNEW"), -1, -1, -1, -1, -1, -1, nil); // Please load or start a new single player or multiplayer game
	DO_HEAP_STUFF();
	CObject::DeleteAllTempObjects();
	DO_HEAP_STUFF();
#ifndef GTA_LIBERTY
	if (!m_bHasSuspended)
#endif
	{
		LoadBaseModels();
		RestoreModels();
		CStreaming::SetFlagsStreamingInfoForModel(0, STREAMFLAGS_DONT_REMOVE);
		CStreaming::RemoveAllUnusedModels();
		CStreaming::RemoveUnusedModelsInLoadedList();
#ifdef GTA_LIBERTY
		CStreaming::RequestModel(MI_PLAYER, STREAMFLAGS_DONT_REMOVE);
#else
		CStreaming::RemoveCurrentZonesModels();
#ifdef FIX_BUGS
		CStreaming::RequestModel(MI_PLAYER, STREAMFLAGS_DONT_REMOVE, "Multi Game");
#else
		CStreaming::RequestModel(MI_PLAYER, STREAMFLAGS_DONT_REMOVE, nil);
#endif
		CStreaming::LoadAllRequestedModels(false);
#endif
	}
	DO_HEAP_STUFF();
	//cSmallHeap::msInstance->DumpContents();
	field_10C = -1;
	gbMP_StartingScriptsFromLua = false;
	//cSmallHeap::msInstance->cSmallHeap_sub_8A39ACC();
	DO_HEAP_STUFF();
	if (!m_bHasSuspended) {
		//cSmallHeap::msInstance->cVolatileRam_sub_89B9BA0(0);
		//cSmallHeap::msInstance->Lock(0);
	}
	DO_HEAP_STUFF();
#ifndef GTA_PC // in psp is disabled, todo figure out collision (+ in psp worldstream build, non pc ipl)
	CStreaming::EnableStreaming();
	DO_HEAP_STUFF();
#endif
	CGarages::RemoveAllCrateGarages();
	DO_HEAP_STUFF();
#ifndef GTA_LIBERTY
	delete m_pGameZoneInfo;
	SetGameZoneInfo(nil);
#endif
	DO_HEAP_STUFF();
#ifdef GTA_TRAIN
	CTrain::NetworkShutdown();
#endif
	TheCamera.SetFadeColour(0, 0, 0);
	DO_HEAP_STUFF();
#ifdef GTA_LIBERTY
	CPlayerPed* pPed = CWorld::Players[CWorld::PlayerInFocus].m_pPed;
	if (pPed != nil) {
		pPed->SetPedState(PedState::PED_IDLE);
		pPed->SetMoveState(eMoveState::PEDMOVE_STILL);
		pPed->m_nLastPedState = PedState::PED_NONE;
	}
#else
	CSwimBoundary::EnableBoundary();
	if (!m_bHasSuspended)
		ShowMenu();
	DO_HEAP_STUFF();
	DO_HEAP_STUFF();
#endif

#ifdef FIX_BUGS
	// PC: New Game,Quit Game - Shutdown +(new game) Initialise (long load), Load Save Game - InitialiseWhenRestarting ShutDownForRestart CWorld::ClearForRestart (del ent)
	// PSP: New Game, Load Save Game - InitialiseWhenRestarting ShutDownForRestart CWorld::ClearForRestart
	// psp bug: after mp in ~sElement deleting CWorld::Players[CWorld::PlayerInFocus].m_pPed, SetupPlayerPed not register RegisterReference,
	// so ped after mp is uaf! On PC if New Game/Quit -> Shutdown -> uaf crash on delete (if(ped)), Load Game with uaf ped not crash [load->scm create ped]
	// in psp create ped before scm in CGame::StartNewGame, so for pc shutdown need valid ped for compat with shutdown and load. its delete in 
	// NewGame after mp - CGame::ShutDown, LoadGame after mp - CWorld::ClearForRestart
	// fix for right deleting in new/load/quit/next mp game
	// from CGame::StartNewGame(), next mp game after mp game will delete this ped in ClearForRestart before create ped, todo mean
	// so i register pPed ref and mg delete it
	CPlayerPed::SetupPlayerPed(0/*, 0*/); // TODO: unk arg 2 // rewrite CWorld::Players[CWorld::PlayerInFocus].m_pPed
#endif
#undef DO_HEAP_STUFF
}

#ifndef GTA_LIBERTY
void cMultiGame::LoadMultiraceGameTypeModels()
{
	int32 score = TheMPGame.GetCTFScoreLimit();
	if (score == 1)
		LoadMultiraceGameTypeCtf1Models();
	else if (score == 2)
		CStreaming::RequestModel(MI_JETSKI, MP_LOAD_FLAGS);
	else
		CStreaming::RequestModel(m_nRaceCarID, MP_LOAD_FLAGS);
}

void cMultiGame::LoadDefendTheBaseGameTypeModels()
{
	switch (GetGameLocation())
	{
		case eGameLocation::VICE_POINT_ZON: // 0
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_STRETCH, MP_LOAD_FLAGS);
					break;
				}
			break;
		}
		case eGameLocation::WASHINGTON_BEACH_ZON: // 1
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
				case 1:
					CStreaming::RequestModel(MI_STRETCH, MP_LOAD_FLAGS);
					break;
				case 2:
					CStreaming::RequestModel(MI_POLICE, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::OCEAN_BEACH_ZON: // 2
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_CHOLLO, MP_LOAD_FLAGS);
					break;
				case 1:
					CStreaming::RequestModel(MI_AMBULAN, MP_LOAD_FLAGS);
					break;
				case 2:
					CStreaming::RequestModel(MI_JETMAX, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::PRAWN_ISLAND_ZON: // 3
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_TOPFUN, MP_LOAD_FLAGS);
					break;
				case 1:
					CStreaming::RequestModel(MI_CADDY, MP_LOAD_FLAGS);
					break;
				case 2:
					CStreaming::RequestModel(MI_HERMES, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::LEAF_LINKS_ZON: // 4 missed
		{
			break;
		}
		case eGameLocation::STARFISH_ISLAND_ZON: // 5
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_STRETCH, MP_LOAD_FLAGS);
					break;
				case 1:
					CStreaming::RequestModel(MI_CHOLLO, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::DOWNTOWN_ZON: // 6
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_STRETCH, MP_LOAD_FLAGS);
					break;
				case 1:
				case 2:
					CStreaming::RequestModel(MI_STRETCH, MP_LOAD_FLAGS);
					break;
				case 3:
					CStreaming::RequestModel(MI_POLMAV, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_FBICAR, MP_LOAD_FLAGS);
					break;
				case 4:
					CStreaming::RequestModel(MI_FIRETRUCK, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::LITTLE_HAITI_ZON: // 7
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0: // equal case 1
					//CStreaming::RequestModel(MI_PATRIOT, MP_LOAD_FLAGS);
					//break;
				case 1:
					CStreaming::RequestModel(MI_PATRIOT, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::LITTLE_HAVANA_ZON: // 8
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_MULE, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_SECURICA, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::VICE_PORT_ZON: // 9
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_SECURICA, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::ESCOBAR_INT_AIRPOIT_ZON: // 10
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_SQUALO, MP_LOAD_FLAGS);
					break;
				case 1:
					CStreaming::RequestModel(MI_SKIMMER, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::FORT_BAXTER_AIRBASE_ZON: // 11
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_RHINO, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
	}
}

void cMultiGame::LoadMultiraceGameTypeCtf1Models()
{
	switch (GetGameLocation())
	{
		case eGameLocation::VICE_POINT_ZON: // 0
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_SANCHEZ, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_INFERNUS, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_HOVERCR, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_HUNTER, MP_LOAD_FLAGS);
					break;
				case 1:
					CStreaming::RequestModel(MI_PIMP, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_PCJ600, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_JETSKI, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_INFERNUS, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::DOWNTOWN_ZON: // 6
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_HERMES, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_PCJ600, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_MESA, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_SPEEDER, MP_LOAD_FLAGS);
					break;
				case 1:
					CStreaming::RequestModel(MI_JETSKI, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_PCJ600, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_JETMAX, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_POLMAV, MP_LOAD_FLAGS);
					break;
				case 2:
					CStreaming::RequestModel(MI_HOVERCR, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_SPEEDER, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_ELECTRAP, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_JETSKI, MP_LOAD_FLAGS);
					break;
			}
		}
	}
}

void cMultiGame::LoadCtfGameTypeModels()
{
	switch (GetGameLocation())
	{
		case eGameLocation::VICE_POINT_ZON: // 0
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_STRETCH, MP_LOAD_FLAGS);
					break;
				case 1:
					CStreaming::RequestModel(MI_POLMAV, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_JETMAX, MP_LOAD_FLAGS);
					break;
				case 2:
				case 5:
					CStreaming::RequestModel(MI_INFERNUS, MP_LOAD_FLAGS);
					break;
				case 3:
				case 6:
					CStreaming::RequestModel(MI_INFERNUS, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_JETMAX, MP_LOAD_FLAGS);
					break;
				case 4:
					CStreaming::RequestModel(MI_AMBULAN, MP_LOAD_FLAGS);
					break;
				case 7:
					CStreaming::RequestModel(MI_POLMAV, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_STRETCH, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::WASHINGTON_BEACH_ZON: // 1
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_POLICE, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_ENFORCER, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::OCEAN_BEACH_ZON: // 2 missed
		{
			break;
		}
		case eGameLocation::PRAWN_ISLAND_ZON: // 3
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_POLMAV, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_JETMAX, MP_LOAD_FLAGS);
					break;
				case 1:
					CStreaming::RequestModel(MI_INFERNUS, MP_LOAD_FLAGS);
					break;
				case 2:
					CStreaming::RequestModel(MI_POLMAV, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_INFERNUS, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::LEAF_LINKS_ZON: // 4 missed
		{
			break;
		}
		case eGameLocation::STARFISH_ISLAND_ZON: // 5
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_JETMAX, MP_LOAD_FLAGS);
					break;
			}
			break;
		}
		case eGameLocation::DOWNTOWN_ZON: // 6
		{
			switch (m_nScenarioOrRaceTrackID)
			{
				case 0:
					CStreaming::RequestModel(MI_RHINO, MP_LOAD_FLAGS);
					break;
				case 1:
					CStreaming::RequestModel(MI_POLMAV, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_HERMES, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_PIMP, MP_LOAD_FLAGS);
					break;
				case 2:
					CStreaming::RequestModel(MI_HERMES, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_PIMP, MP_LOAD_FLAGS);
					break;
				case 3:
					CStreaming::RequestModel(MI_POLMAV, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_STRETCH, MP_LOAD_FLAGS);
					break;
				case 4: // like 2
					CStreaming::RequestModel(MI_HERMES, MP_LOAD_FLAGS);
					CStreaming::RequestModel(MI_PIMP, MP_LOAD_FLAGS);
					break;
			}
			break;
		}

	// unused
	//case eGameLocation::LITTLE_HAITI_ZON: // 7
	//case eGameLocation::LITTLE_HAVANA_ZON: // 8
	//case eGameLocation::VICE_PORT_ZON: // 9
	//case eGameLocation::ESCOBAR_INT_AIRPOIT_ZON: // 10
	//case eGameLocation::FORT_BAXTER_AIRBASE_ZON: // 11
	//	break;
	}
}
#endif

#ifdef GTA_LIBERTY
// TODO IsElementExists(sElement*)
bool cMultiGame::IsLocalElement(sElement* pElement)
{
	uint8 nPeerID = LocalPlayerID();
	if (m_vPlayers[nPeerID]->m_vElements.size() == 0) return false;
	for (std::map<uint16, sElement*>::iterator it = m_vPlayers[nPeerID]->m_vElements.begin(); it != m_vPlayers[nPeerID]->m_vElements.end(); it++) {
		if (it->second != nil && it->second == pElement) return true;
	}
	return false;
}
#else
bool cMultiGame::IsElementExistsForPeer(uint8 nPeerID, sElement* pElement)
{
	return PeerManager.GetPeerById(nPeerID)->IsElementExists(pElement);
}
#endif


void mp_register_waypoint_packets(sWaypoint* pWaypointManager) {
	cMultiGame& Game = cMultiGame::Instance();
#define REGISTER_PACKET(id, callback, arg) Game.RegisterPacket(id, new cPacketDispatcherWaypoint(callback, arg));

	REGISTER_PACKET(gtMP_PacketIDs.set_waypoint.pckt_id, (PcktRecvHndWaypoint)&sWaypoint::OnSetWaypoint, pWaypointManager); // 52
	REGISTER_PACKET(gtMP_PacketIDs.clear_waypoint.pckt_id, (PcktRecvHndWaypoint)&sWaypoint::OnClearWaypoint, pWaypointManager); // 53
	REGISTER_PACKET(gtMP_PacketIDs.hit_waypoint.pckt_id, (PcktRecvHndWaypoint)&sWaypoint::OnHitWaypoint, pWaypointManager); // 54
#undef REGISTER_PACKET
}

void mp_regiter_packets() {
	cMultiGame& Game = cMultiGame::Instance();
#define REGISTER_PACKET(id, callback) Game.RegisterPacket(id, new cPacketDispatcher(callback));
#define REGISTER_IMPROVEMENTS

#if 1 // my normal sort
	REGISTER_PACKET(gtMP_PacketIDs.start_fire.pckt_id, &on_recv_start_fire); // 0
	//REGISTER_PACKET(gtMP_PacketIDs.ack.pckt_id, &on_recv_ack); // 1  not cb, internal stuff
	//REGISTER_PACKET(gtMP_PacketIDs.info.pckt_id, &on_recv_info); // 2  not cb, internal stuff
	//REGISTER_PACKET(gtMP_PacketIDs.player_kill.pckt_id, &on_recv_player_kill); // 3  cMultiGame::Open()
	REGISTER_PACKET(gtMP_PacketIDs.kick_player.pckt_id, &on_recv_kick_player); // 4
#if defined(REGISTER_IMPROVEMENTS) || defined(GTA_LIBERTY)
	REGISTER_PACKET(gtMP_PacketIDs.request_kick_player.pckt_id, &on_recv_request_kick_player); // 5  unused in vcs
#endif
	REGISTER_PACKET(gtMP_PacketIDs.set_team_score.pckt_id, &on_recv_set_team_score); // 6
	REGISTER_PACKET(gtMP_PacketIDs.send_game_event.pckt_id, &on_recv_send_game_event); // 7
	REGISTER_PACKET(gtMP_PacketIDs.force_ped_from_vehicle.pckt_id, &on_recv_force_ped_from_vehicle); // 8
	REGISTER_PACKET(gtMP_PacketIDs.set_vehicle_emergency_break_state.pckt_id, &on_recv_set_vehicle_emergency_break_state); // 9
	REGISTER_PACKET(gtMP_PacketIDs.set_carlocked_state.pckt_id, &on_recv_set_carlocked_state); // 10
	REGISTER_PACKET(gtMP_PacketIDs.repair_car.pckt_id, &on_recv_repair_car); // 11
	REGISTER_PACKET(gtMP_PacketIDs.set_tyres_no_burst.pckt_id, &on_recv_set_tyres_no_burst); // 12
	REGISTER_PACKET(gtMP_PacketIDs.delete_vehicle.pckt_id, &on_recv_delete_vehicle); // 13
	//REGISTER_PACKET(gtMP_PacketIDs.game_state.pckt_id, &on_recv_game_state); // 14  cMultiGame::Open()
	REGISTER_PACKET(gtMP_PacketIDs.play_remote_sound.pckt_id, &on_recv_play_remote_sound); // 15
	//REGISTER_PACKET(gtMP_PacketIDs.heart_beat.pckt_id, &on_recv_heart_beat); // 16  no cb  <<----------------------------
	//REGISTER_PACKET(gtMP_PacketIDs.transfer_entity.pckt_id, &on_recv_transfer_entity); // 17  cMultiGame::Open()
	//REGISTER_PACKET(gtMP_PacketIDs.clock.pckt_id, &on_recv_clock); // 18  unused by game
	REGISTER_PACKET(gtMP_PacketIDs.game_time.pckt_id, &on_recv_game_time); // 19
	REGISTER_PACKET(gtMP_PacketIDs.target_player.pckt_id, &on_recv_target_player); // 20
	//REGISTER_PACKET(gtMP_PacketIDs.set_fixed_camera.pckt_id, &on_recv_set_fixed_camera); // 21  lscript_open_camera()
	//REGISTER_PACKET(gtMP_PacketIDs.restore_camera.pckt_id, &on_recv_restore_camera); // 22  lscript_open_camera()
	REGISTER_PACKET(gtMP_PacketIDs.pickup_collected.pckt_id, &on_recv_pickup_collected); // 23
	REGISTER_PACKET(gtMP_PacketIDs.pickup_request.pckt_id, &on_recv_pickup_request); // 24
	REGISTER_PACKET(gtMP_PacketIDs.powerup_collected.pckt_id, &on_recv_powerup_collected); // 25
	REGISTER_PACKET(gtMP_PacketIDs.set_vehicle_infinite_mass.pckt_id, &on_recv_set_vehicle_infinite_mass); // 26
	REGISTER_PACKET(gtMP_PacketIDs.fight_hit_ped.pckt_id, &on_recv_fight_hit_ped); // 27
	REGISTER_PACKET(gtMP_PacketIDs.shot_ped.pckt_id, &on_recv_shot_ped); // 28
	REGISTER_PACKET(gtMP_PacketIDs.fire_instant_hit.pckt_id, &on_recv_fire_instant_hit); // 29
	REGISTER_PACKET(gtMP_PacketIDs.fire_sniper.pckt_id, &on_recv_fire_sniper); // 30
	REGISTER_PACKET(gtMP_PacketIDs.fire_shotgun.pckt_id, &on_recv_fire_shotgun); // 31
	REGISTER_PACKET(gtMP_PacketIDs.fire_projectile.pckt_id, &on_recv_fire_projectile); // 32
	REGISTER_PACKET(gtMP_PacketIDs.use_detonator.pckt_id, &on_recv_use_detonator); // 33
	REGISTER_PACKET(gtMP_PacketIDs.fire_area_effect.pckt_id, &on_recv_fire_area_effect); // 34
	REGISTER_PACKET(gtMP_PacketIDs.shot_ped_from_car.pckt_id, &on_recv_shot_ped_from_car); // 35
	REGISTER_PACKET(gtMP_PacketIDs.fire_instant_hit_car.pckt_id, &on_recv_fire_instant_hit_car); // 36
	REGISTER_PACKET(gtMP_PacketIDs.kill_player_ped.pckt_id, &on_recv_kill_player_ped); // 37
	REGISTER_PACKET(gtMP_PacketIDs.shot_vehicle.pckt_id, &on_recv_shot_vehicle); // 38
	REGISTER_PACKET(gtMP_PacketIDs.melee.pckt_id, &on_recv_melee); // 39
	REGISTER_PACKET(gtMP_PacketIDs.player_been_hit.pckt_id, &on_recv_player_been_hit); // 40   why not in lscript_open_player?
	//REGISTER_PACKET(gtMP_PacketIDs.player_control.pckt_id, &on_recv_player_control); // 41  lscript_open_player()
	//REGISTER_PACKET(gtMP_PacketIDs.set_position.pckt_id, &on_recv_player_set_position); // 42  lscript_open_player()
	//REGISTER_PACKET(gtMP_PacketIDs.set_heading.pckt_id, &on_recv_player_set_heading); // 43  lscript_open_player()
	REGISTER_PACKET(gtMP_PacketIDs.add_3d_marker.pckt_id, &on_recv_add_3d_marker); // 44
	REGISTER_PACKET(gtMP_PacketIDs.remove_3d_marker.pckt_id, &on_recv_remove_3d_marker); // 45
	//REGISTER_PACKET(gtMP_PacketIDs.enable_roads.pckt_id, &on_recv_enable_roads); // 46  lscript_open_roads()
	//REGISTER_PACKET(gtMP_PacketIDs.clear_area.pckt_id, &on_recv_clear_area); // 47  lscript_open_roads()
	REGISTER_PACKET(gtMP_PacketIDs.set_player_blip_visible_state.pckt_id, &on_recv_set_player_blip_visible_state); // 48
	REGISTER_PACKET(gtMP_PacketIDs.player_respawn.pckt_id, &on_recv_player_respawn); // 49
	REGISTER_PACKET(gtMP_PacketIDs.add_explosion.pckt_id, &on_recv_add_explosion); // 50
	//REGISTER_PACKET(gtMP_PacketIDs.print_now.pckt_id, &on_recv_print_now); // 51  lscript_open_text_sprite()
	//REGISTER_PACKET(gtMP_PacketIDs.set_waypoint.pckt_id, &on_recv_set_waypoint); // 52  mp_register_waypoint_packets()
	//REGISTER_PACKET(gtMP_PacketIDs.clear_waypoint.pckt_id, &on_recv_clear_waypoint); // 53  mp_register_waypoint_packets()
	//REGISTER_PACKET(gtMP_PacketIDs.hit_waypoint.pckt_id, &on_recv_hit_waypoint); // 54  mp_register_waypoint_packets()
	REGISTER_PACKET(gtMP_PacketIDs.spawn_car_debris.pckt_id, &on_recv_spawn_car_debris); // 55
	REGISTER_PACKET(gtMP_PacketIDs.set_vehicle_health.pckt_id, &on_recv_set_vehicle_health); // 56
	REGISTER_PACKET(gtMP_PacketIDs.set_vehicle_position.pckt_id, &on_recv_set_vehicle_position); // 57
	REGISTER_PACKET(gtMP_PacketIDs.vehicle_impact.pckt_id, &on_recv_vehicle_impact); // 58
	//REGISTER_PACKET(gtMP_PacketIDs.msg_ready_for_cutscene.pckt_id, &on_recv_msg_ready_for_cutscene); // 59  lscript_open_matching()
#ifndef GTA_LIBERTY
	REGISTER_PACKET(gtMP_PacketIDs.ack_entity_create.pckt_id, &on_recv_ack_entity_create); // 60
	REGISTER_PACKET(gtMP_PacketIDs.sync_peer_group.pckt_id, &on_recv_sync_peer_group); // 61
	REGISTER_PACKET(gtMP_PacketIDs.debug_break.pckt_id, &on_recv_debug_break); // 62
	REGISTER_PACKET(gtMP_PacketIDs.msg_blowup_vehicle.pckt_id, &on_recv_msg_blowup_vehicle); // 63
	REGISTER_PACKET(gtMP_PacketIDs.msg_create_lua_object.pckt_id, &on_recv_msg_create_lua_object); // 64
	REGISTER_PACKET(gtMP_PacketIDs.msg_server_ready_to_go.pckt_id, &on_recv_msg_server_ready_to_go); // 65
#endif
#else // psp/leeds bad sort
	REGISTER_PACKET(gtMP_PacketIDs.sync_peer_group.pckt_id, &on_recv_sync_peer_group); // 61
	REGISTER_PACKET(gtMP_PacketIDs.play_remote_sound.pckt_id, &on_recv_play_remote_sound); // 15
	REGISTER_PACKET(gtMP_PacketIDs.send_game_event.pckt_id, &on_recv_send_game_event); // 7
	REGISTER_PACKET(gtMP_PacketIDs.set_team_score.pckt_id, &on_recv_set_team_score); // 6
	REGISTER_PACKET(gtMP_PacketIDs.force_ped_from_vehicle.pckt_id, &on_recv_force_ped_from_vehicle); // 8
	REGISTER_PACKET(gtMP_PacketIDs.set_vehicle_emergency_break_state.pckt_id, &on_recv_set_vehicle_emergency_break_state); // 9
	REGISTER_PACKET(gtMP_PacketIDs.set_carlocked_state.pckt_id, &on_recv_set_carlocked_state); // 10
	REGISTER_PACKET(gtMP_PacketIDs.repair_car.pckt_id, &on_recv_repair_car); // 11
	REGISTER_PACKET(gtMP_PacketIDs.delete_vehicle.pckt_id, &on_recv_delete_vehicle); // 13
	REGISTER_PACKET(gtMP_PacketIDs.set_tyres_no_burst.pckt_id, &on_recv_set_tyres_no_burst); // 12
	REGISTER_PACKET(gtMP_PacketIDs.game_time.pckt_id, &on_recv_game_time); // 19
	REGISTER_PACKET(gtMP_PacketIDs.target_player.pckt_id, &on_recv_target_player); // 20
	REGISTER_PACKET(gtMP_PacketIDs.kick_player.pckt_id, &on_recv_kick_player); // 4
	REGISTER_PACKET(gtMP_PacketIDs.fight_hit_ped.pckt_id, &on_recv_fight_hit_ped); // 27
	REGISTER_PACKET(gtMP_PacketIDs.shot_ped.pckt_id, &on_recv_shot_ped); // 28
	REGISTER_PACKET(gtMP_PacketIDs.shot_ped_from_car.pckt_id, &on_recv_shot_ped_from_car); // 35
	REGISTER_PACKET(gtMP_PacketIDs.fire_instant_hit.pckt_id, &on_recv_fire_instant_hit); // 29
	REGISTER_PACKET(gtMP_PacketIDs.fire_sniper.pckt_id, &on_recv_fire_sniper); // 30
	REGISTER_PACKET(gtMP_PacketIDs.fire_shotgun.pckt_id, &on_recv_fire_shotgun); // 31
	REGISTER_PACKET(gtMP_PacketIDs.fire_projectile.pckt_id, &on_recv_fire_projectile); // 32
	REGISTER_PACKET(gtMP_PacketIDs.use_detonator.pckt_id, &on_recv_use_detonator); // 33
	REGISTER_PACKET(gtMP_PacketIDs.fire_area_effect.pckt_id, &on_recv_fire_area_effect); // 34
	REGISTER_PACKET(gtMP_PacketIDs.fire_instant_hit_car.pckt_id, &on_recv_fire_instant_hit_car); // 36
	REGISTER_PACKET(gtMP_PacketIDs.shot_vehicle.pckt_id, &on_recv_shot_vehicle); // 38
	REGISTER_PACKET(gtMP_PacketIDs.melee.pckt_id, &on_recv_melee); // 39
	REGISTER_PACKET(gtMP_PacketIDs.pickup_request.pckt_id, &on_recv_pickup_request); // 24
	REGISTER_PACKET(gtMP_PacketIDs.pickup_collected.pckt_id, &on_recv_pickup_collected); // 23
	REGISTER_PACKET(gtMP_PacketIDs.powerup_collected.pckt_id, &on_recv_powerup_collected); // 25
	REGISTER_PACKET(gtMP_PacketIDs.add_explosion.pckt_id, &on_recv_add_explosion); // 50
	REGISTER_PACKET(gtMP_PacketIDs.start_fire.pckt_id, &on_recv_start_fire); // 0
	REGISTER_PACKET(gtMP_PacketIDs.spawn_car_debris.pckt_id, &on_recv_spawn_car_debris); // 55
	REGISTER_PACKET(gtMP_PacketIDs.set_vehicle_health.pckt_id, &on_recv_set_vehicle_health); // 56
	REGISTER_PACKET(gtMP_PacketIDs.set_vehicle_position.pckt_id, &on_recv_set_vehicle_position); // 57
	REGISTER_PACKET(gtMP_PacketIDs.msg_blowup_vehicle.pckt_id, &on_recv_msg_blowup_vehicle); // 63
	REGISTER_PACKET(gtMP_PacketIDs.add_3d_marker.pckt_id, &on_recv_add_3d_marker); // 44
	REGISTER_PACKET(gtMP_PacketIDs.remove_3d_marker.pckt_id, &on_recv_remove_3d_marker); // 45
	REGISTER_PACKET(gtMP_PacketIDs.player_been_hit.pckt_id, &on_recv_player_been_hit); // 40   why not in lscript_open_player?
	REGISTER_PACKET(gtMP_PacketIDs.vehicle_impact.pckt_id, &on_recv_vehicle_impact); // 58
	REGISTER_PACKET(gtMP_PacketIDs.kill_player_ped.pckt_id, &on_recv_kill_player_ped); // 37
	REGISTER_PACKET(gtMP_PacketIDs.set_player_blip_visible_state.pckt_id, &on_recv_set_player_blip_visible_state); // 48
	REGISTER_PACKET(gtMP_PacketIDs.player_respawn.pckt_id, &on_recv_player_respawn); // 49
	REGISTER_PACKET(gtMP_PacketIDs.set_vehicle_infinite_mass.pckt_id, &on_recv_set_vehicle_infinite_mass); // 26
	REGISTER_PACKET(gtMP_PacketIDs.ack_entity_create.pckt_id, &on_recv_ack_entity_create); // 60
	REGISTER_PACKET(gtMP_PacketIDs.debug_break.pckt_id, &on_recv_debug_break); // 62
	REGISTER_PACKET(gtMP_PacketIDs.msg_create_lua_object.pckt_id, &on_recv_msg_create_lua_object); // 64
	REGISTER_PACKET(gtMP_PacketIDs.msg_server_ready_to_go.pckt_id, &on_recv_msg_server_ready_to_go); // 65
#if defined(REGISTER_IMPROVEMENTS) || defined(GTA_LIBERTY)
	REGISTER_PACKET(gtMP_PacketIDs.request_kick_player.pckt_id, &on_recv_request_kick_player); // 5  unused in vcs
#endif
	// todo missed id + ifndef GTA_LIBERTY 59+
#endif
#undef REGISTER_IMPROVEMENTS
#undef REGISTER_PACKET
}

/* TODO: better place for this (in LCS exec it's in a strange module with lua natives) */
void loadWeapons() {
	CStreaming::LoadAllRequestedModels(false);
	CStreaming::FlushRequestList();
	debug("Loading weapons for MP game\n");
#ifdef GTA_LIBERTY
	CStreaming::RequestModel(MI_MOBILE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_BASEBALL_BAT, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_CHAINSAW, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_GRENADE, STREAMFLAGS_DONT_REMOVE); // huh?
	CStreaming::RequestModel(MI_GRENADE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_MOLOTOV, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_MISSILE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_COLT45, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_SHOTGUN, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_TEC9, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_RUGER, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_ROCKETLAUNCHER, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_MINIGUN, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_SNIPERRIFLE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_BOMB, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_PICKUP_HEALTH, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_PICKUP_BODYARMOUR, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_PICKUP_KILLFRENZY, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_PICKUP_MEGADAMAGE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_PICKUP_REGENERATOR, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_PICKUP_INVISIBLE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_PICKUP_GOOD_CAR, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_PICKUP_BAD_CAR, STREAMFLAGS_DONT_REMOVE);
#else
	//CStreaming::RequestModel(MI_MOBILE, STREAMFLAGS_DONT_REMOVE); // lcs
	CStreaming::RequestModel(MI_BASEBALL_BAT, STREAMFLAGS_DONT_REMOVE);
	//CStreaming::RequestModel(MI_CHAINSAW, STREAMFLAGS_DONT_REMOVE); // lcs
	if (TheMPGame.GetGameType() == eGameType::MULTIRACE)
	{
		CStreaming::RequestModel(MI_PICKUP_GOOD_CAR, STREAMFLAGS_DONT_REMOVE);
		CStreaming::RequestModel(MI_PICKUP_BAD_CAR, STREAMFLAGS_DONT_REMOVE);
		int32 score = TheMPGame.GetCTFScoreLimit();
		if (score == 1 || score == 2)
			return;
	}
	CStreaming::RequestModel(MI_GRENADE, STREAMFLAGS_DONT_REMOVE); // huh?
	CStreaming::RequestModel(MI_GRENADE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_MOLOTOV, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_MISSILE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_COLT45, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_SHOTGUN, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_SKOR, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_AK47, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_ROCKETLAUNCHER, STREAMFLAGS_DONT_REMOVE);
	//CStreaming::RequestModel(MI_MINIGUN, STREAMFLAGS_DONT_REMOVE); // lcs
	CStreaming::RequestModel(MI_SNIPERRIFLE, STREAMFLAGS_DONT_REMOVE);
	CStreaming::RequestModel(MI_BOMB, STREAMFLAGS_DONT_REMOVE);
	if (TheMPGame.GetGameType() != eGameType::MULTIRACE)
	{
		CStreaming::RequestModel(MI_PICKUP_HEALTH, STREAMFLAGS_DONT_REMOVE);
		CStreaming::RequestModel(MI_PICKUP_BODYARMOUR, STREAMFLAGS_DONT_REMOVE);
		CStreaming::RequestModel(MI_PICKUP_KILLFRENZY, STREAMFLAGS_DONT_REMOVE);
		CStreaming::RequestModel(MI_PICKUP_MEGADAMAGE, STREAMFLAGS_DONT_REMOVE);
		CStreaming::RequestModel(MI_PICKUP_REGENERATOR, STREAMFLAGS_DONT_REMOVE);
		CStreaming::RequestModel(MI_PICKUP_INVISIBLE, STREAMFLAGS_DONT_REMOVE);
	}
#endif
	CStreaming::LoadAllRequestedModels(false);
}


// RECV - CORELOOP - SEND - RENDER
// begin frame
void mp_game_update_recv() {
#if !defined(FINAL) && !defined(MASTER)
	tbStartTimer(0, "cMultiGame::UpdateReceive");
#endif
#ifdef DEBUG_MULTIGAME_IMPROVEMENTS
	gAllowCreateElement = true;
#endif
	MG_LAG_START(1);
	cMultiGame::Instance().UpdateReceive();
	MG_LAG_END(1, "RECV", 0.1);
#if !defined(FINAL) && !defined(MASTER)
	tbEndTimer("cMultiGame::UpdateReceive");
#endif
}

// end frame (next render stage)
void mp_game_update_send() {
#if !defined(FINAL) && !defined(MASTER)
	tbStartTimer(0, "cMultiGame::UpdateSend");
#endif
	MG_LAG_START(1);
	cMultiGame::Instance().UpdateSend();
	MG_LAG_END(1, "SEND", 0.1);
#ifdef DEBUG_MULTIGAME_IMPROVEMENTS
	gAllowCreateElement = false;
#endif
#if !defined(FINAL) && !defined(MASTER)
	tbEndTimer("cMultiGame::UpdateSend");
#endif
}

void mp_game_draw_debug_net() {
	cMultiGame& Game = cMultiGame::Instance();
	if (!Game.m_pNetSession)
		return;

	CFont::SetFontStyle(FONT_STANDARD);
	CFont::SetBackgroundOff();
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetWrapx(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetScale(0.85f, 1.0f);
	CFont::SetCentreOff();
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetJustifyOff();
	CFont::SetColor(CRGBA(50, 80, 180, 255));
	CFont::SetDropShadowPosition(1);
	CFont::SetPropOn();
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));

	float x = 16.0f;
	float y = 250.0f;

	char line[512];
	wchar wline[512];
	uint16 basis = Game.GetPlayerPed(Game.LocalPlayerID()) ? Game.GetElementOwnerZone(Game.GetPlayerPed(Game.LocalPlayerID()))->m_nBasis : 0;
	sprintf(line, "[ NS m_vPeers %d m_nPeerCount %d ]  [ PM m_vPlayers %d ] DELTAS/MS %f / %f FRAME %d ACK %d",
		Game.m_pNetSession->m_vPeers.size(),
		Game.m_pNetSession->m_nPeerCount,
		PeerManager.m_vPlayers.size(),
		Game.m_pNetSession->m_Timer.fDeltaS,
		Game.m_pNetSession->m_Timer.fDeltaMs,
		basis,
		FindPlayerZoneMG()->m_vAck.size());

	AsciiToUnicode(line, wline);
	CFont::PrintString(x, y, wline);
}

void mp_game_log_zone_stuff(cInterestZone* pZone)
{
	printf("ZONE %d\n", pZone->m_nID);
	for (uint32 i = 0; i < pZone->m_vElements.size(); i++) {
		printf("  [%d] %d %s\n", i, pZone->m_vElements[i]->GetID(), GetElementStringType(pZone->m_vElements[i]));
	}
	if (!pZone->m_vElements.size())
		printf("  EMPTY!\n");
}

void mp_game_draw_debug_zones() {
	cMultiGame& Game = cMultiGame::Instance();
	if (!Game.m_pNetSession)
		return;

	CFont::SetFontStyle(FONT_STANDARD);
	CFont::SetBackgroundOff();
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetWrapx(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetScale(0.85f, 1.0f);
	CFont::SetCentreOff();
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetJustifyOff();
	CFont::SetColor(CRGBA(50, 80, 180, 255));
	CFont::SetDropShadowPosition(1);
	CFont::SetPropOn();
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));

	float x = 16.0f;
	float y = 250.0f;
	float lineHeight = 20.0f;

	char line[512];
	wchar wline[512];
	sPed* ped = FindPlayerPedMG();
	sprintf(line, "[NETSESSION] %d  [GAMEFRAME] %d sPed TIME %d LAG %d",
		Game.m_pNetSession->m_nCurTime, CTimer::GetFrameCounter(), ped ? ped->m_nTime : 0, Game.m_nLagValue);
	AsciiToUnicode(line, wline);
	CFont::PrintString(x, y, wline);
	y += lineHeight;

	std::map<uint16, cInterestZone*> vZones = Game.m_ZoneManager.GetZones();
	cInterestZone* pPlayerZone = Game.GetElementOwnerZone(Game.GetPlayerPed(Game.LocalPlayerID()));
	for (auto& pair : vZones) {
		uint16 id = pair.first;
		cInterestZone* pZone = pair.second;
		if (pZone) {
			//mp_game_log_zone_stuff(pZone);

			if (pPlayerZone && pZone->m_nID == pPlayerZone->m_nID) {
				CFont::SetColor(CRGBA(255, 20, 20, 255));
				sprintf(line, "[PLAYER] %d - m_nBasis %d - m_nCurTime %d - ELEMS %d",
					pZone->m_nID, pZone->m_nBasis, pZone->m_nCurTime, pZone->m_vElements.size());
			}
			else {
				CFont::SetColor(CRGBA(50, 80, 180, 255));
				sprintf(line, "[OTHER] %d - m_nBasis %d - m_nCurTime %d - ELEMS %d",
					pZone->m_nID, pZone->m_nBasis, pZone->m_nCurTime, pZone->m_vElements.size());
			}

			AsciiToUnicode(line, wline);
			CFont::PrintString(x, y, wline);
			y += lineHeight;

			if (y > SCREEN_SCALE_Y(DEFAULT_SCREEN_HEIGHT) - 50) {
				break;
			}
		}
	}
}

void* allocFunc(uint32 size) {
	return operator new(size); // return base::cMainMemoryManager::Instance()->Allocate(size); // op overload
}

void deleteFunc(void* buff) {
	operator delete(buff); // base::cMainMemoryManager::Instance()->Free(buff); // op overload
}

int32 FindPlayerHostID() {
	return NET_SESSION_DEFAULT_HOST_ID;
#if 0
	cMultiGame& Game = cMultiGame::Instance();
	if (!gIsMultiplayerGame || !Game.m_pNetSession) return -1;
	tMacAddr hostMac = TheAdhoc.GetMatchingInfo(MP_HOST_INDEX)->m_HostPeerData.peerAddr.mac;

	for (int32 nPeerID = 0; nPeerID < (int32)Game.m_pNetSession->m_vPeers.size(); ++nPeerID)
	{
		cNetPeerState* pPeer = Game.m_pNetSession->m_vPeers[nPeerID];
		if (!pPeer) continue;
		sPeerState* peerState = PeerManager.GetPeerById(pPeer->m_nPeerId);
		if (peerState && peerState->m_Addr.mac == hostMac) return peerState->m_nID;
	}
	return -1;
#endif
}

#ifndef GTA_PSP
double mp_time_now_d() {
	auto now = std::chrono::steady_clock::now().time_since_epoch();
	return std::chrono::duration<double>(now).count();
}
#endif

void ClearMultiplayerSplashScreen()
{
	gbMultiplayerSplash = false;
	//gbLevelSplash = false;
	TODO();
}

cNetConfig::cNetConfig() {
	sNickName = "EBLAN";
	bIsDynamicName = false;
	bMACNickName = false;
#ifndef GTA_PSP // emu
	sProAdhocServer = "localhost"; // +dns resolve 127.0.0.1
	bIsDynamicProAdhocServerAddr = false;
	sMACAddress = CreateRandMAC();
	INFO_LOG(Log::sceNet, "Generated mac: %s", sMACAddress.c_str());
	bEnableAdhocServer = true;
	iPortOffset = 10000;
	bEnableWlan = true;
	bEnableNetworkChat = true;
	bPtpPdpDedicatedEmu = false;
	nPtpPdpDedicatedEmuChannel = 0;
	bPtpPdpDedicatedEmuChatChannel = false;

	bEnableUPnP = false;
	bUPnPUseOriginalPort = false;
	iMinTimeout = 0;
#endif
}

void cNetConfig::Init() {
	uint8_t mac[SCE_NET_ETHER_ADDR_LEN] = { 0 };

	if (GAME_ID > 1) {
		assert(!IsUnCaseContains(sProAdhocServer, "LANIP")); // can't listen same P2P port at multiinstance, fine using port offset, but aemu doesn't send port offset
		// localhost is shift local addr by id in sceNetAdhocPdpCreate - getLocalIp - InitLocalhostIP

		// generate instance mac
		memset(mac, GAME_ID, SCE_NET_ETHER_ADDR_LEN);
		// Making sure the 1st 2-bits on the 1st byte of OUI are zero to prevent issue with some games (ie. Gran Turismo)
		mac[0] &= 0xfc;
		g_Config.sMACAddress = MacToString(mac);
		INFO_LOG(Log::sceNet, "Changed to instance mac: %s", sMACAddress.c_str());

		// generate instance name
		if (!bIsDynamicName) { // already dynamic
			sNickName += ("_" + std::to_string(GAME_ID));
			bIsDynamicName = true;
			INFO_LOG(Log::sceNet, "Changed to instance nickname: %s", sNickName.c_str());
		}
	}

	if (IsUnCaseContains(sProAdhocServer, "LANIP")) {
		sProAdhocServer = net::GetOutboundIP();
		bIsDynamicProAdhocServerAddr = true;
		INFO_LOG(Log::sceNet, "Changed to Outbound IP: %s", sProAdhocServer.c_str());
	}
	assert(!IsUnCaseContains(sProAdhocServer, "LANIP"));

	if (bMACNickName) {
		sNickName = sMACAddress;
		bIsDynamicName = true;
		INFO_LOG(Log::sceNet, "Changed to MAC nickname: %s", sNickName.c_str());
	}
}

cNetConfig g_Config;

//int16 GetPeerFromPlayerElement(sPlayer* player) {
//	int16 nIndex = 0;
//	cMultiGame& Game = cMultiGame::Instance();
//	if (Game.GetPlayerCount() <= 0)
//		return 0;
//	while (Game.GetPlayer(nIndex) != player) {
//		nIndex++;
//		if (nIndex >= Game.GetPlayerCount())
//			return 0;
//	}
//	return nIndex;
//}

inline bool HandleConnectionError(bool bNoPeersConnected, bool bIsOneTeamEmpty, bool abort = false) {
	cAdhoc& Adhoc = TheAdhoc;
	cMultiGame& Game = TheMPGame;
#ifdef GTA_LIBERTY
	if (Game.m_bHasSuspended)
#else
	if ((abort || Adhoc.HadError()) || Game.m_bHasSuspended)
#endif
	{
		CGame::AbortMultiplayerSession(bNoPeersConnected, bIsOneTeamEmpty);
		return true;
	}
	return false;
}

// true - continue CGame::Process // after ProcessMultiGame need be world focused ped for FindPlayerPed()
/*inline*/ bool ProcessMultiGame()
{
//#if !defined(FINAL) && !defined(MASTER)
//#define LOADINGSCREEN(s1, s2, splash, bMultiGameChunksNum)
//#else
#define LOADINGSCREEN(s1, s2, splash, bMultiGameChunksNum) LoadingScreen(s1, s2, splash, bMultiGameChunksNum)
//#endif

	cAdhoc& Adhoc = TheAdhoc;
	cMultiGame& Game = TheMPGame;

#ifdef GTA_PC
	AdhocEmu_NetUpdate(); // emu ppsspp fake threads psp context for update matching events, probably we can native this in std::thread, test it
#endif

	if (!g_bInitMPTime)
	{
		g_bInitMPTime = true;
#ifdef GTA_PSP
		sceKernelSysClock clock = sceKernelGetSystemTimeWide();
		uint32 sec, usec;
		sceKernelSysClock2USecWide(clock, &sec, &usec);
		g_fMPPrevTime = static_cast<float>(sec) + (usec / 1000000.0f);
#else
		g_fMPPrevTime = mp_time_now_d();
#endif
	}

	if (!gIsMultiplayerGame)
	{
#ifdef GTA_PSP
		sceKernelSysClock clock = sceKernelGetSystemTimeWide();
		uint32 sec, usec;
		sceKernelSysClock2USecWide(clock, &sec, &usec);
		g_fMPPrevTime = static_cast<float>(sec) + (usec / 1000000.0f);
#else
		g_fMPPrevTime = mp_time_now_d();
#endif
		return true; // <<---- base return to SP
	}

	bool abort = false;
	bool bNoPeersConnected = false;
	bool bIsOneTeamEmpty = false;

	if (Game.IsOpen() && Game.m_bIsConnected && !Adhoc.bConnEvent)
	{
		debug("Adhoc Connection Screwed Aborting\n");
		abort = true;
	}

#ifdef GTA_LIBERTY
	if (!Game.m_bIsConnected || Game.LocalPlayerID() != 0 || Game.GetNumberOfPeersConnected() != 0)
#else
	if (!Game.m_bIsConnected || Game.LocalPlayerID() != 0 || PeerManager.m_vPlayers.size() >= MULTI_ABORT_PLAYER_COUNT)
#endif
	{
		if (Game.m_bIsConnected && GANG_MODE && Game.IsAnyTeamEmpty())
		{
			debug("There are no players on one of the teams, Aborting\n");
			abort = true;
			bIsOneTeamEmpty = true;
		}
	}
	else
	{
		debug("Num Peers Connected == 0, Aborting\n");
		abort = true;
		bNoPeersConnected = true;
	}

	//if (Adhoc.HadError())
	//	abort = true;

	//if (abort || Game.m_bHasSuspended) {
	//	debug("if (abort || Game.m_bHasSuspended)\n"); // custom
	//	CGame::AbortMultiplayerSession(bNoPeersConnected, bIsOneTeamEmpty);
	//	return false;
	//}

	if (HandleConnectionError(bNoPeersConnected, bIsOneTeamEmpty, abort))
		return false;

	if (!Game.IsOpen())
	{
		debug("MultiGame not open... opening it\n");
		Adhoc.Update();
		if (!Adhoc.bConnEvent)
		{
#ifdef GTA_PSP
			sceKernelSysClock clock = sceKernelGetSystemTimeWide();
			uint32 sec, usec;
			sceKernelSysClock2USecWide(clock, &sec, &usec);
			float currentTime = static_cast<float>(sec) + (usec / 1000000.0f);
#else
			float currentTime = mp_time_now_d();
#endif

			float elapsed = currentTime - g_fMPPrevTime;
			if (elapsed > MULTI_TIME_OUT_4)
			{
#ifdef GTA_PSP
				sceKernelSysClock clock2 = sceKernelGetSystemTimeWide();
				uint32 sec2, usec2;
				sceKernelSysClock2USecWide(clock2, &sec2, &usec2);
				float timeoutTime = static_cast<float>(sec2) + (usec2 / 1000000.0f);
#else
				float timeoutTime = mp_time_now_d();
#endif
				debug("***** Multigame timeout with time %f\n", (timeoutTime - g_fMPPrevTime));
				CGame::AbortMultiplayerSession(true, false);
				return false;
			}

#ifdef GTA_PSP
			sceKernelSysClock clock3 = sceKernelGetSystemTimeWide();
			uint32 sec3, usec3;
			sceKernelSysClock2USecWide(clock3, &sec3, &usec3);
			g_fMPPrevTime = static_cast<float>(sec3) + (usec3 / 1000000.0f);
#else
			g_fMPPrevTime = mp_time_now_d();
#endif
			return false;
		}

		// good, proceed
		LOADINGSCREEN("MULTI GAME", nil, nil, true);
		tListenAddr listenAddr;
		//listenAddr.mac.InitMacAddr(); // in ctor
		//listenAddr.port = NET_SESSION_DEFAULT_PORT; // in ctor
		Game.Open(); // probably listenAddr is arg, in lcs log cMultiGame::Open() without args, or beta stuff, or old time log, or vcs upd unused beta stuff
//#ifdef GTA_LIBERTY
//		if (Game.m_bHasSuspended) // recheck
//#else
//		if (Adhoc.HadError())
//			abort = true;
//
//		if (abort || Game.m_bHasSuspended)
//#endif
//		{
//			CGame::AbortMultiplayerSession(false, false);
//			return false;
//		}
		if (HandleConnectionError(false, false, abort))
			return false;
		LOADINGSCREEN("MULTI GAME", nil, nil, true);

		if (!Game.Connect()) {
			debug("Failed to connect players...\n");
			CGame::AbortMultiplayerSession(false, false);
			return false;
		}
		LOADINGSCREEN("MULTI GAME", nil, nil, true);
	}

//#ifdef GTA_LIBERTY
//	if (Game.m_bHasSuspended)
//#else
//	if (Adhoc.HadError())
//		abort = true;
//
//	if (abort || Game.m_bHasSuspended)
//#endif
//	{
//		CGame::AbortMultiplayerSession(false, false);
//		return false;
//	}

	if (HandleConnectionError(false, false, abort))
		return false;

	if (!Game.m_bIsConnected)
	{
		debug("MultiGame not connected.... connecting it\n");

		if (Game.PerformInitialConnection())
		{
//#ifdef GTA_LIBERTY
//			if (Game.m_bHasSuspended)
//				return false;
//#else
//			if (Adhoc.HadError())
//				abort = true;
//
//			if (abort || Game.m_bHasSuspended)
//			{
//				CGame::AbortMultiplayerSession(false, false);
//				return false;
//			}
//#endif
			if (HandleConnectionError(false, false, abort))
				return false;

			debug("Loading stage 1\n");
			LOADINGSCREEN("MULTI GAME", nil, nil, true);
			TheHud->GetRidOfAllHudMessages();
//#ifdef GTA_LIBERTY
//			if (Game.m_bHasSuspended)
//				return false;
//#else
//			if (Adhoc.HadError())
//				abort = true;
//
//			if (abort || Game.m_bHasSuspended)
//			{
//				CGame::AbortMultiplayerSession(false, false);
//				return false;
//			}
//#endif
			if (HandleConnectionError(false, false, abort))
				return false;

			debug("Loading stage 2\n");
			LOADINGSCREEN("MULTI GAME", nil, nil, true);
			// some beta load here!
//#ifdef GTA_LIBERTY
//			if (Game.m_bHasSuspended)
//				return false;
//#else
//			if (Adhoc.HadError())
//				abort = true;
//
//			if (abort || Game.m_bHasSuspended)
//			{
//				CGame::AbortMultiplayerSession(false, false);
//				return false;
//			}
//
//			// why twice beta load?
//			if (Adhoc.HadError())
//				abort = true;
//
//			if (abort || Game.m_bHasSuspended)
//			{
//				CGame::AbortMultiplayerSession(false, false);
//				return false;
//			}
//#endif
			if (HandleConnectionError(false, false, abort))
				return false;

			// why twice beta load?
			if (HandleConnectionError(false, false, abort))
				return false;

			CTheScripts::Init(false); // TODO VCS: when fix load game sequence bigload init/ quick InitialiseWhenRestarting
//#ifdef GTA_LIBERTY
//			if (Game.m_bHasSuspended)
//				return false;
//#else
//			if (Adhoc.HadError())
//				abort = true;
//
//			if (abort || Game.m_bHasSuspended)
//			{
//				CGame::AbortMultiplayerSession(false, false);
//				return false;
//			}
//#endif
			if (HandleConnectionError(false, false, abort))
				return false;

			debug("Loading stage 3\n");
			LOADINGSCREEN("MULTI GAME", nil, nil, true);
			CTheScripts::Process(); // same
//#ifdef GTA_LIBERTY
//			if (Game.m_bHasSuspended)
//#else
//			if (Adhoc.HadError())
//				abort = true;
//
//			if (abort || Game.m_bHasSuspended)
//#endif
//			{
//				CGame::AbortMultiplayerSession(false, false);
//				return false;
//			}

			if (HandleConnectionError(false, false, abort))
				return false;

			debug("Loading stage 4\n");
			LOADINGSCREEN("MULTI GAME", nil, nil, true);
			Game.LoadScene();
			CStreaming::LoadScene(TheCamera.GetPosition());
//#ifdef GTA_LIBERTY
//			if (Game.m_bHasSuspended)
//				return false;
//#else
//			if (Adhoc.HadError())
//				abort = true;
//
//			if (abort || Game.m_bHasSuspended)
//			{
//				CGame::AbortMultiplayerSession(false, false);
//				return false;
//			}
//#endif
			if (HandleConnectionError(false, false, abort))
				return false;

			debug("Loading stage 5\n");
			LOADINGSCREEN("MULTI GAME", nil, nil, true);
			return true; // <<---------- ok End MG init
		} // perform

//#ifdef GTA_LIBERTY
//		if (Game.m_bHasSuspended)
//			return false;
//#else
//		if (Adhoc.HadError())
//			abort = true;
//
//		if (abort || Game.m_bHasSuspended)
//		{
//			CGame::AbortMultiplayerSession(false, false);
//			return false;
//		}
//#endif
		HandleConnectionError(false, false, abort);

		return false; // <<------ end CGame::Process VCS
	} // conn

	return true;
#undef LOADINGSCREEN
}

cMultiGame cMultiGame::msInstance;
uint8 cMultiGame::s_nPlayerModelIndex = 0;
eGameTeam cMultiGame::s_nSelectedTeam = eGameTeam::TEAM_A;


// TODO: find another place to declare this
tGangDef gMPGangDefs[MP_MAX_GANGS] = {
#ifdef GTA_LIBERTY
    {"GNG8", 0},
    {"GNG9", 0},
    {"GNG1", 0},
    {"GNG2", 1},
    {"GNG3", 1},
    {"GNG4", 2},
    {"GNG5", 2},
    {"GNG0", 3},
    {"GNG7", 3},
    {"GNG6", 3},
#else // VCS Unlocked
    {"GNG0", 0},
    {"GNG1", 0},
	{"GNG2", 0},
	{"GNG3", 0},
	{"GNG4", 0},
	{"GNG5", 0},
	{"GNG6", 0},
	{"GNG7", 0},
    {"GNG8", 0},
    {"GNG9", 0},
#endif
};

#ifndef GTA_LIBERTY
int32 gMPGangDefsNetModelListIndices[MP_MAX_GANGS] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
#endif

bool gMPCutsceneHasPlayed[7] = {
    false, false, false,
    false, false, false,
    false
};

#ifndef GTA_LIBERTY
int32 gMPScenarioNumsTable[MAX_SCENARIO_NUMS_TYPES][static_cast<int32>(eGameLocation::NUM_MP_GAME_LOCATION)] = // also RaceTrackNums
{
  { 2, 2, 1, 0, 0, 0, 1, 2, 1, 1, 2, 0 }, // ST_MULTIRACE_CTF_SCORE_0
  { 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0 }, // ST_MULTIRACE_CTF_SCORE_1
  { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // ST_MULTIRACE_CTF_SCORE_2
  { 0, 1, 2, 2, 0, 1, 1, 1, 1, 1, 1, 1 }, // ST_DEFENDTHEBASE
  { 4, 0, 0, 0, 0, 1, 3, 1, 1, 0, 0, 1 }, // ST_CTF
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 }, // ST_TANK
  { 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 }, // ST_SIXTYSECONDS
  { 8, 3, 2, 4, 1, 2, 9, 6, 5, 3, 0, 1 }, // ST_HUNTERATTACK
  { 3, 2, 0, 0, 0, 0, 1, 4, 1, 1, 0, 0 }, // ST_FLAGBALL
  { 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 }, // ST_VIP
};
#endif
