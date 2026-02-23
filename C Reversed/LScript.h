/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "common.h"

#include "leeds/base/stringt.h"
#include "multiplayer/MultiGame.h" // for DEBUG_MULTIGAME

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#ifdef GTA_PSP
	#define PSP_DEV_USERDIR "host0:./"
	#define PSP_USERDIR "DISC0:/PSP_GAME/USRDIR"
	#ifdef GTA_LIBERTY
		#define GTA_LUA_SCRIPTDIR PSP_USERDIR"/LUASCRIPTS"
	#else // GTA_MIAMI
		#define GTA_LUA_SCRIPTDIR PSP_USERDIR"/LUA"
	#endif
	#define GTA_LUA_PATH GTA_LUA_SCRIPTDIR"/?.LUA.LC;" \
		GTA_LUA_SCRIPTDIR"/?.LUA;" \
		GTA_LUA_SCRIPTDIR"/?;" \
		PSP_USERDIR"/?"
#else
	#ifdef GTA_LIBERTY
		#define GTA_LUA_SCRIPTDIR "luascripts"
	#else
		#define GTA_LUA_SCRIPTDIR "lua"
	#endif
	//#ifdef USE_COMPILED_LUA
	//	#define GTA_LUA_PATH GTA_LUA_SCRIPTDIR"/?.lua.lc"
	//#else
	//	#define GTA_LUA_PATH GTA_LUA_SCRIPTDIR"/?.lua"
	//#endif
	#define GTA_LUA_PATH GTA_LUA_SCRIPTDIR"/?.lua;" \
		GTA_LUA_SCRIPTDIR"/?.lua.lc;"
#endif

#ifdef DEBUG_MULTIGAME
	#define LUA_PRINT_ENABLED (1)
#endif

#define LUA_TABLE_BEGIN() \
	lua_newtable(L);

#define LUA_PUSH_CONSTANT(constname, value) \
	lua_pushstring(L, constname); \
	lua_pushnumber(L, value); \
	lua_settable(L, -3);

#define LUA_PUSH_CONSTANT_VEC(constname, x, y, z) \
	lua_pushstring(L, constname); \
	ls_pushVector3D(L, x, y, z); \
	lua_settable(L, -3);

#define LUA_TABLE_END(tablename) \
	lua_pushstring(L, tablename); \
	lua_insert(L, -2); \
	lua_settable(L, LUA_GLOBALSINDEX);

#ifdef GTA_LIBERTY
#define VALIDATE_LUA_LIB(lib, szvcs, szlcs) static_assert(ARRAY_SIZE(lib) == szlcs, "Wrong LCS table size")
#else
#define VALIDATE_LUA_LIB(lib, szvcs, szlcs) static_assert(ARRAY_SIZE(lib) == szvcs, "Wrong VCS table size")
#endif

class cLScript {
public:
	static void Initialize();
#ifdef DEBUG_MULTIGAME
	static void InitializeDebug(const char* lua_file);
#endif
	static void Shutdown();
	static void RunMainScript();
};

/* TODO: this class extends cSingleton */
class cLWrapper {
private:
	static cLWrapper* msInstance;
public:
	lua_State* m_luaVM;
	void Open();
	void Close();
	void CreateGlobalLibrary(const luaL_reg* libconfig, const char* libname);
	void CreateLibrary(const luaL_reg* libconfig, const char* tablename);
	void DoFile(base::string& path);
	void ExecString(const base::string& script);
	base::string ExecStringResult(const base::string& script); // custom
	cLWrapper();

	static cLWrapper& Instance()
	{
		if (!msInstance)
			msInstance = new cLWrapper;
		return *msInstance;
	}
};

bool HasLuaInitialized();
