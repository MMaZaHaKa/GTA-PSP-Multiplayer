/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "General.h"
#include "Streaming.h"
#include "CutsceneMgr.h"
#include "Frontend.h"
#include "Script.h"

#include "multiplayer/MultiGame.h"
#include "multiplayer/natives/public.h"
#include "multiplayer/events/public.h"


int lsn_hasModelLoaded(lua_State* L) {
	int nModelID = luaL_checknumber(L, 1);
	lua_pushboolean(L, CStreaming::HasModelLoaded(nModelID));
	return 1;
}

int lsn_requestModel(lua_State* L) {
	int nModelID = luaL_checknumber(L, 1);
	CStreaming::RequestModel(nModelID, STREAMFLAGS_NOFADE | STREAMFLAGS_DEPENDENCY | STREAMFLAGS_SCRIPTOWNED);
	lua_pushboolean(L, CStreaming::HasModelLoaded(nModelID));
	return 1;
}

inline void open_model(lua_State* L) {
	lua_register(L, "RequestModel",   lsn_requestModel);
	lua_register(L, "HasModelLoaded", lsn_hasModelLoaded);
}

int lsn_loadCutscene(lua_State* L) {
	const char* name = luaL_checklstring(L, 1, NULL);
#ifdef GTA_LIBERTY
	CCutsceneMgr::LoadCutsceneData(name);
#else
	TheCutscene->LoadCutsceneData(name);
#endif
	return 0;
}

int lsn_isCutsceneLoaded(lua_State* L) {
#ifdef GTA_LIBERTY
	lua_pushboolean(L, CCutsceneMgr::ms_cutsceneLoadStatus == CUTSCENE_LOADED);
#else
	lua_pushboolean(L, TheCutscene->m_cutsceneLoadStatus == CUTSCENE_LOADED);
#endif
	return 1;
}

int lsn_startCutscene(lua_State* L) {
#ifdef GTA_LIBERTY
	CCutsceneMgr::StartCutscene();
#else
	TheCutscene->StartCutscene();
#endif
	return 0;
}

int lsn_hasCutsceneFinished(lua_State* L) {
#ifdef GTA_LIBERTY
	lua_pushboolean(L, CCutsceneMgr::HasCutsceneFinished());
#else
	lua_pushboolean(L, TheCutscene->HasCutsceneFinished());
#endif
	return 1;
}

int lsn_clearCutscene(lua_State* L) {
	debug("Clear lua cutscene\n");
#ifdef GTA_LIBERTY
	CCutsceneMgr::DeleteCutsceneData();
#else
	TheCutscene->DeleteCutsceneData();
#endif
	CTheScripts::Shutdown();
	return 0;
}

int lsn_clearSplashScreen(lua_State* L) {
	debug("Clear splash screen\n");
	ClearMultiplayerSplashScreen();
	return 0;
}

int lsn_runScriptedCutscene(lua_State* L) {
	debug("run scripted cutscene\n");
	gbMP_StartingScriptsFromLua = true;
	CTheScripts::Init(false);
	CTheScripts::StartTestScript();
	return 0;
}

int lsn_processScriptedCutscene(lua_State* L) {
	CTheScripts::Process();
	lua_pushboolean(L, gbMP_StartingScriptsFromLua);
	return 1;
}

inline void open_cutscene(lua_State* L) {
	lua_register(L, "LOAD_CUTSCENE",           lsn_loadCutscene);
	lua_register(L, "IS_CUTSCENE_LOADED",      lsn_isCutsceneLoaded);
	lua_register(L, "START_CUTSCENE",          lsn_startCutscene);
	lua_register(L, "HAS_CUTSCENE_FINISHED",   lsn_hasCutsceneFinished);
	lua_register(L, "CLEAR_CUTSCENE",          lsn_clearCutscene);
	lua_register(L, "ClearSplashScreen",       lsn_clearSplashScreen);
	lua_register(L, "RunScriptedCutscene",     lsn_runScriptedCutscene);
	lua_register(L, "ProcessScriptedCutscene", lsn_processScriptedCutscene);
}

int mp_lsn_setVectorForSinglePlayerScript(lua_State* L) {
	CVector vec;
	lsc_getVectorFromStack(vec, L, 1, true);
	gVecForSinglePlayerScript = vec;
	return 0;
}

int mp_lsn_getAngleBetweenPoints(lua_State* L) {
	float nAngle, x1, y1, x2, y2;
	x1 = lua_tonumber(L, 1);
	y1 = lua_tonumber(L, 2);
	x2 = lua_tonumber(L, 3);
	y2 = lua_tonumber(L, 4);
	nAngle = CGeneral::GetAngleBetweenPoints(x1, y1, x2, y2);
	lua_pushnumber(L, nAngle);
	return 1;
}

int mp_lsn_crossProduct(lua_State* L) {
	CVector v1, v2, v3;
	lsc_getVectorFromStack(v1, L, 1, true);
	lsc_getVectorFromStack(v2, L, 2, true);
	v3 = CrossProduct(v1, v2);
	lsc_pushVuVector(L, v3);
	return 1;
}

int mp_lsn_vecNormalise(lua_State* L) {
	CVector vec;
	lsc_getVectorFromStack(vec, L, 1, true);
	vec.Normalise();
	lsc_pushVuVector(L, vec);
	return 1;
}

int mp_lsn_vecScale(lua_State* L) {
	CVector vec;
	lsc_getVectorFromStack(vec, L, 1, true);
	const int nScale = lua_tonumber(L, 1);
	vec.x *= nScale;
	vec.y *= nScale;
	vec.z *= nScale;
	lsc_pushVuVector(L, vec);
	return 1;
}

inline void open_math(lua_State* L) {
	lua_register(L, "SetVectorForSinglePlayerScript", mp_lsn_setVectorForSinglePlayerScript);
	lua_register(L, "GetAngleBetweenPoints", mp_lsn_getAngleBetweenPoints);
	lua_register(L, "CrossProduct", mp_lsn_crossProduct);
	lua_register(L, "VecNormalise", mp_lsn_vecNormalise);
	lua_register(L, "VecScale", mp_lsn_vecScale);
}

int mp_lsn_distance(lua_State* L) {
	CVector v1, v2;
	lsc_getVectorFromStack(v1, L, 1, true);
	lsc_getVectorFromStack(v2, L, 2, true);
	const float nDistance = Distance(v1, v2);
	lua_pushnumber(L, nDistance);
	return 1;
}

int mp_lsn_distance2D(lua_State* L) {
	CVector v1, v2;
	lsc_getVectorFromStack(v1, L, 1, true);
	lsc_getVectorFromStack(v2, L, 2, true);
	const float nDistance = Distance2D(v1, v2);
	lua_pushnumber(L, nDistance);
	return 1;
}

int mp_lsn_addVectors(lua_State* L) {
	CVector v1, v2, res;
	lsc_getVectorFromStack(v1, L, 1, true);
	lsc_getVectorFromStack(v2, L, 2, true);
	res = v1 + v2;
	lsc_pushVuVector(L, res);
	return 1;
}

inline void open_vector(lua_State* L) {
	lua_register(L, "Distance",   mp_lsn_distance);
	lua_register(L, "Distance2D", mp_lsn_distance2D);
	lua_register(L, "AddVectors", mp_lsn_addVectors);
}


int mp_lsn_SetTeamScore(lua_State* L) {
	cMultiGame& Game = TheMPGame;
	const int nTeamID = lua_tonumber(L, 1);
	const int nScore = lua_tonumber(L, 2);
	net::pckt_set_team_score packet;
	packet.pckt_size = sizeof(net::pckt_set_team_score);
	packet.pckt_id = gtMP_PacketIDs.set_team_score.pckt_id;
	packet.score = nScore;
	packet.team_id = nTeamID;
	Game.m_anTeamScore[nTeamID] = nScore;
	Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
#ifdef FIX_BUGS
	return 0;
#else
	return 1;
#endif
}

int mp_lsn_getTeamScore(lua_State* L) {
	const int32 nTeamID = lua_tonumber(L, 1);
	const int32 nScore = TheMPGame.m_anTeamScore[nTeamID];
	lua_pushnumber(L, nScore);
	return 1;
}

int mp_lsn_isEventStackEmpty(lua_State* L) {
	lua_pushboolean(L, TheMPGame.m_pEventStack->isEmpty());
	return 1;
}

int mp_lsn_getEvent(lua_State* L) {
	cEventStack* pStack = TheMPGame.m_pEventStack;
	if (!pStack->isEmpty()) {
		lua_pushnumber(L, pStack->pop());
	}
	else {
		lua_pushnil(L);
	}
	return 1;
}

void lsc_send_event(int id) {
	cMultiGame& Game = TheMPGame;
	net::pckt_send_game_event packet;
	packet.pckt_size = sizeof(net::pckt_send_game_event);
	packet.pckt_id = gtMP_PacketIDs.send_game_event.pckt_id;
	packet.event = id;
	Game.SendMessagePriority(packet, BROADCAST_PEER_GROUPID);
	on_recv_send_game_event(packet, Game.LocalPlayerID(), 0, true);
}

int mp_lsn_sendEvent(lua_State* L) {
	if (lua_isnumber(L, -1)) lsc_send_event(lua_tonumber(L, -1));
	return 0;
}

/* unused script function */
int mp_lsn_clearEventStack(lua_State* L) {
	debug("clearing event stack\n");
	TheMPGame.m_pEventStack->clear();
	return 0;
}

int mp_lsn_maskEventNumber(lua_State* L) {
	uint32 nA, nB, nMask;
	int32 nC;

	nA = lua_tonumber(L, 1);
	nB = lua_tonumber(L, 2);
	nC = lua_tonumber(L, 3);

	nMask = nA & nB;

	if (nC > 0) {
		if (nC >= 32u) nMask = 0;
		else nMask = nMask << nC;
	}
	else if (nC < 0) {
		int32 s = -nC;
		if (s >= 32u) nMask = 0;
		else nMask = nMask >> s;
	}
	lua_pushnumber(L, nMask);
	return 1;
}

int mp_lsn_setTeamTimer(lua_State* L) {
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) return 0;
	int32 nTeamID = lua_tonumber(L, 1);
	int32 nTimer = lua_tonumber(L, 2);
	TheMPGame.m_anTeamTimer[nTeamID] = nTimer;
	return 0;
}

int mp_lsn_getTeamTimer(lua_State* L) {
	if (!lua_isnumber(L, 1)) return 0;
	int32 nTeamID = lua_tonumber(L, 1);
	lua_pushnumber(L, TheMPGame.m_anTeamTimer[nTeamID]);
	return 1;
}

int mp_lsn_setTeamTimerCounting(lua_State* L) {
	if (!lua_isnumber(L, 1)) return 0;
	if (lua_isboolean(L, 2)) {
		int32 nTeamID = lua_tonumber(L, 1);
		bool toggle = lua_toboolean(L, 2);
		TheMPGame.m_abTeamTimerEnabled[nTeamID] = toggle;
	}
	return 0;
}

int mp_lsn_isSphereOnScreen(lua_State* L) {
	CVector pos;
	cMultiGame& pGame = cMultiGame::Instance();
	lsc_getVectorFromStack(pos, L, 1, true);
	float fRadius = lua_tonumber(L, 2);
	float fMaxDist = lua_tonumber(L, 3);
	for (int32 id = 0; id < pGame.GetPlayersCount(); ++id)
	{
		sPlayer* pPlayer = pGame.GetPlayer(id);
		if (pPlayer && pPlayer->isPositionInRadius(pos, fRadius, fMaxDist))
		{
			lua_pushboolean(L, true);
			return 1;
		}
	}
	lua_pushboolean(L, false);
	return 1;
}

int mp_lsn_doMemoryPrint(lua_State* L) {
	debug("mp_lsn_doMemoryPrint\n"); // custom, probably heap dump mem
	return 0;
}

int mp_lsn_dumpStreamingContents(lua_State* L) {
	CStreaming::PrintStreamingBufferStateToConsole();
	return 0;
}

#ifndef GTA_LIBERTY
int mp_lsn_UseLCFiles(lua_State* L) {
	lua_pushboolean(L, !gbIsUsingLUASource); // is using compiled lua
	return 1;
}

int mp_lsn_GetLanguage(lua_State* L) {
	lua_pushnumber(L, FrontEndMenuManager->m_PrefsLanguage); // VCS PSP cLangSettings
	return 1;
}
#endif

static const luaL_reg ls_others_lib[] = {
	{"SetTeamScore",          mp_lsn_SetTeamScore},
	{"GetTeamScore",          mp_lsn_getTeamScore},
	{"IsEventStackEmpty",     mp_lsn_isEventStackEmpty},
	{"GetEvent",              mp_lsn_getEvent},
	{"SendEvent",             mp_lsn_sendEvent},
	{"ClearEventStack",       mp_lsn_clearEventStack},
	{"MaskEventNumber",       mp_lsn_maskEventNumber},
	{"SetTeamTimer",          mp_lsn_setTeamTimer},
	{"GetTeamTimer",          mp_lsn_getTeamTimer},
	{"SetTeamTimerCounting",  mp_lsn_setTeamTimerCounting},
	{"IsSphereOnScreen",      mp_lsn_isSphereOnScreen},
	{"DoMemoryPrint",         mp_lsn_doMemoryPrint},
	{"DumpStreamingContents", mp_lsn_dumpStreamingContents},
#ifndef GTA_LIBERTY
	{"UseLCFiles",            mp_lsn_UseLCFiles},
	{"GetLanguage",           mp_lsn_GetLanguage},
#endif
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_others_lib, (15 + 1), (13 + 1));

void lscript_open_others() {
	cLWrapper& wrapper = cLWrapper::Instance();
	wrapper.CreateGlobalLibrary(ls_others_lib, nil); // openlib
}

void lscript_open_main() {
	cLWrapper& wrapper = cLWrapper::Instance();
	lua_State* pVM = wrapper.m_luaVM;
	open_vector(pVM);
	lscript_open_entity();
	lscript_open_player();
	lscript_open_vehicle();
	lscript_open_radar();
	lscript_open_text_sprite();
#ifndef GTA_LIBERTY
	lscript_open_net2dMeter();
#endif
	lscript_open_waypoints();
	lscript_open_halo();
	lscript_open_pickups();
	lscript_open_garages();
	lscript_open_camera();
	lscript_open_roads();
	lscript_open_others();
	open_model(pVM);
	open_cutscene(pVM);
	open_math(pVM);
}
