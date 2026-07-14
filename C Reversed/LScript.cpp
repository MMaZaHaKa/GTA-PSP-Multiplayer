/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/LScript.h"
#include "multiplayer/natives/public.h"
#include "multiplayer/MultiGame.h"
#include "FileMgr.h"
#include "main.h"
#include <time.h> // for rnd

static bool hasLuaInitialized = false;

bool HasLuaInitialized() {
	return hasLuaInitialized;
}

inline base::string getScriptFile(const char* name, bool autoext = true) {
	base::string value = base::string(GTA_LUA_SCRIPTDIR "/");
	value.Append(name);
	if (autoext)
	{
		value.Append(".lua");
		if (!gbIsUsingLUASource) // compiled
			value.Append(".lc");
	}
	return value;
}

void cLScript::Initialize() {
	debug("LUA INIT!\n");
	cLWrapper& pWrapper = cLWrapper::Instance();
	pWrapper.Open();
	lscript_open_matching();
	lscript_open_cheats();
	lscript_open_constants();
	lscript_open_main();
	lscript_open_simsch();
	lua_State* L = pWrapper.m_luaVM;
	lua_pushstring(L, GTA_LUA_PATH);
	lua_pushstring(L, "LUA_PATH");
	lua_insert(L, -2);
	lua_settable(L, LUA_GLOBALSINDEX);
	base::string file = "";
	// PSP gbIsUsingLUASource if else
	switch (TheMPGame.GetGameType()) {
		case eGameType::DEATHMATCH: file = getScriptFile("deathmatch"); break;
		case eGameType::MULTIRACE: file = getScriptFile("multirace"); break;
		case eGameType::DEFENDTHEBASE: file = getScriptFile("defendthebase"); break;
		case eGameType::CTF: file = getScriptFile("capturetheflag"); break;
		case eGameType::TANK: file = getScriptFile("tank"); break;
		case eGameType::HITPARADE: file = getScriptFile("hitparade"); break;
		case eGameType::SIXTYSECONDS: file = getScriptFile("sixtyseconds"); break;
#ifndef GTA_LIBERTY
		case eGameType::HUNTERATTACK: file = getScriptFile("HunterAttack"); break;
		case eGameType::FLAGBALL: file = getScriptFile("FlagBall"); break;
		case eGameType::VIP: file = getScriptFile("Vip"); break;
#endif
		default: assert(false && "Unknown game type");
	}
	debug("[MP]: Selected LUA: %s\n", file.c_str());
#ifdef FIX_BUGS
	int32 h = CFileMgr::OpenFile(file.c_str(), "rb");
	if (h != 0)
		CFileMgr::CloseFile(h);
	else {
		assert(false && "LUA File not found");
		return;
	}
#endif
	pWrapper.DoFile(file);
	hasLuaInitialized = true;
}

void cLScript::Shutdown() {
	debug("LUA CLOSE!\n");
	cLWrapper& pWrapper = cLWrapper::Instance();
	if (pWrapper.m_luaVM) {
		hasLuaInitialized = false;
		pWrapper.Close();
	}
}

void cLScript::RunMainScript() {
	cLWrapper& pWrapper = cLWrapper::Instance();
	lua_settop(pWrapper.m_luaVM, 0);
	pWrapper.ExecString("Main()");
}

cLWrapper* cLWrapper::msInstance = nil;

cLWrapper::cLWrapper() {
	m_luaVM = nil;
}

/* looks like this was copied from lua.c */
static const luaL_reg luastdlibs[] = {
	{"base",   luaopen_base},
	{"table",  luaopen_table},
	{"string", luaopen_string},
	{"math",   luaopen_math},
	{"debug",  luaopen_debug},
	{NULL, NULL}
};
VALIDATE_LUA_LIB(luastdlibs, (5 + 1), (5 + 1));

int lsn_print(lua_State* L) {
	// vanila: empty

	if (LUA_PRINT_ENABLED) {
		const char* line = lua_tostring(L, -1);
		if (line) {
			SetConsoleColor(1);
			debug("[LUA]: %s\n", line);
			SetConsoleColor(6);
		}
	}
	return 0;
}

#ifdef DEBUG_MULTIGAME
void cLScript::InitializeDebug(const char* lua_file)
{
	assert(lua_file);
	debug("LUA INIT!\n");
	cLWrapper& pWrapper = cLWrapper::Instance();
	pWrapper.Open();
	lscript_open_matching();
	lscript_open_cheats();
	lscript_open_constants();
	lscript_open_main();
	lscript_open_simsch();
	//lscript_open_debug();
	lua_State* L = pWrapper.m_luaVM;
	lua_pushstring(L, GTA_LUA_PATH);
	lua_pushstring(L, "LUA_PATH");
	lua_insert(L, -2);
	lua_settable(L, LUA_GLOBALSINDEX);
	base::string file = "";
	file = getScriptFile(lua_file, false);
	pWrapper.DoFile(file);
	hasLuaInitialized = true;
}
#endif

void cLWrapper::Open() {
#ifdef FIX_BUGS
	srand(time(NULL)); // non vanila, my hotfix(same start rnd) for lua math_random (lmathlib.c), leeds modified lua for base::random
#endif
	m_luaVM = lua_open();
	const luaL_reg* lib = luastdlibs;
	for (; lib && lib->func; lib++) {
		lib->func(m_luaVM);
		lua_settop(m_luaVM, 0);
	}
	lua_register(m_luaVM, "print", lsn_print);
}

void cLWrapper::Close() {
	lua_close(m_luaVM);
	m_luaVM = nil;
}

void cLWrapper::CreateGlobalLibrary(const luaL_reg* libconfig, const char* libname) {
	lua_State* L = m_luaVM;
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_openlib(L, libname, libconfig, 0);
	lua_settop(L, -2);
}

void cLWrapper::CreateLibrary(const luaL_reg* libconfig, const char* tablename) {
	lua_State* L = m_luaVM;
	luaL_newmetatable(L, tablename); // t
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_rawset(L, -3); // t["__index"] = t
	luaL_openlib(L, nil, libconfig, 0);
	lua_settop(L, -2);
}

void logLuaError(const char* tag, const char* msg) {
	// TODO: looks like each class can have it's logger function, but let's keep simple for now
	SetConsoleColor(!strcmp(msg, "(error with no message)")); // 0 red 1 green
	debug("%s: %s\n", tag, msg);
	SetConsoleColor(6);
}

void handleExecError(int errCode) {
	lua_State* L = cLWrapper::Instance().m_luaVM;
	const char* errMsg = lua_tostring(L, -1);
	if (errMsg == nil) errMsg = "(error with no message)";
	logLuaError("gta3", errMsg);
	lua_settop(L, -2);
}

base::string handleExecErrorString(int errCode) {
	lua_State* L = cLWrapper::Instance().m_luaVM;
	const char* errMsg = lua_tostring(L, -1);
	if (errMsg == nil) errMsg = "(error with no message)";
	base::string err = base::string(errMsg);
	lua_settop(L, -2);
	return err;
}

void cLWrapper::DoFile(base::string& path) {
	int errCode = lua_dofile(m_luaVM, path.c_str());
	handleExecError(errCode);
}

void cLWrapper::ExecString(const base::string& script) {
	int errCode = lua_dostring(m_luaVM, script.c_str());
	handleExecError(errCode);
}

base::string cLWrapper::ExecStringResult(const base::string& script) {
	int errCode = lua_dostring(m_luaVM, script.c_str());
	return handleExecErrorString(errCode);
}