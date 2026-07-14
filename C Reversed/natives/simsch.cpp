/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/natives/public.h"
#include "Font.h"


/* accepts one parameter which is a lua function to be executed

Params:
	func - the lua function to be runned
Returns:
	thread - the lua thread created for the function
*/
int lsn_simsch_start(lua_State* L) {
	lua_pushstring(L, "simsch");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_insert(L, 1);
	if (!lua_tothread(L, 2)) {
		lua_State* thread = lua_newthread(L);
		if (!lua_isfunction(L, 2) || lua_iscfunction(L, 2)) {
			luaL_argerror(L, 2, "Lua function expected");
		}
		lua_pushvalue(L, 2);
		lua_xmove(L, thread, 1); // moves the function to the new thread
	}
	// table.append(simsch.actions, {cond=true, script=thread})
	lua_newtable(L); // action
	// stack: simsch, func, thread, action*
	lua_pushstring(L, "cond");
	lua_pushboolean(L, 1);
	lua_settable(L, -3);
	lua_pushstring(L, "script");
	lua_pushvalue(L, -3);
	lua_settable(L, -3);
	lua_pushstring(L, "actions");
	lua_gettable(L, 1); // sets stack top to simsch.actions
	int size = luaL_getn(L, -1);
	luaL_setn(L, -1, size + 1);
	lua_pushvalue(L, -2);
	lua_rawseti(L, -2, size + 1); // actions[actions.size+1] = action
	lua_settop(L, -3); // pops action, actions*
	return 1;
}

int lsn_simsch_stop(lua_State* L) {
	lua_pushstring(L, "simsch");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_insert(L, 1);
	lua_State* thread = lua_tothread(L, 2);
	if (!thread) {
		luaL_argerror(L, 2, "Lua coroutine expected");
	}
	lua_settop(L, 1);
	// stack: simsch*
	lua_pushstring(L, "script");
	lua_pushstring(L, "actions");
	lua_gettable(L, 1);
	lua_pushnil(L);
	// stack: simsch, "script", simsch.actions, nil*
	while (lua_next(L, 3) != 0) { // for _, action in pairs(simsch.actions) do
		lua_pushvalue(L, 2);
		lua_gettable(L, -2); // thread1 = action.script
		if (lua_tothread(L, -1) == thread) { // if found the thread to stop
			lua_pushstring(L, "dead");
			lua_pushboolean(L, true);
			// stack: ..., simsch.actions, key, action, thread1, "dead", true*
			lua_settable(L, -4);
			lua_pushboolean(L, 1);
			return 1;
		}
		lua_settop(L, -3);
	}
	lua_pushboolean(L, false);
	return 1;
}

int lsn_simsch_step(lua_State* L) {
	lua_pushstring(L, "simsch");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_insert(L, 1);
	lua_settop(L, 1);
	lua_pushstring(L, "dead");
	lua_pushstring(L, "cond");
	lua_pushstring(L, "script");
	lua_pushstring(L, "actions");
	lua_gettable(L, 1);
	lua_pushnil(L);
	while (lua_next(L, 5) != 0) { // for _, action in pairs(simsch.actions) do
		lua_pushvalue(L, 2);
		lua_gettable(L, -2); // isDead = action.dead
		bool isDead = lua_toboolean(L, -1);
		lua_settop(L, -2); // pops isDead
		if (!isDead) {
			bool can_execute = false;
			int stack_top = lua_gettop(L);
			lua_pushvalue(L, 3);
			lua_gettable(L, -2); // cond = action.cond
			if (!lua_isboolean(L, -1)) lua_call(L, 0, LUA_MULTRET);
			if (stack_top < lua_gettop(L)) can_execute = lua_toboolean(L, stack_top + 1);
			if (can_execute) {
				lua_pushvalue(L, 4);
				lua_gettable(L, stack_top); // action.script
				lua_State* thread = lua_tothread(L, -1);
				lua_settop(L, -2); // pops action.script
				int args_count = lua_gettop(L) - stack_top - 1; // number of extra arguments excluding the boolean
				if (args_count <= 0) args_count = 0;
				else lua_xmove(L, thread, args_count);
				int err_code = lua_resume(thread, args_count);
				bool isValid = (
					!lua_isboolean(thread, -1) ||
					(lua_toboolean(thread, -1) && !lua_isnil(thread, -1))
				);
				if (!err_code && lua_gettop(thread) && isValid) {
					lua_pushvalue(L, 3);
					lua_xmove(thread, L, 1);
					lua_settable(L, stack_top); // action.cond = yield thread
					lua_settop(thread, 0);
				}
				else {
					lua_pushvalue(L, 2);
					lua_pushboolean(L, true);
					lua_settable(L, stack_top); // action.dead = true
					if (err_code) {
						lua_xmove(thread, L, 1);
						return lua_error(L);
					}
				}
			}
			lua_settop(L, stack_top);
		}
		lua_settop(L, -2);
	}
	int key = 1;
	int index = 1;
	// remove those which are dead, fill the remaining space with nil
	while (true) {
		lua_rawgeti(L, 5, key); // action = simsch.actions[i]
		if (!lua_istable(L, -1)) break;
		key++;
		lua_pushvalue(L, 2);
		lua_gettable(L, -2); // action.dead
		int is_dead = lua_toboolean(L, -1);
		lua_settop(L, -2);
		if (is_dead) lua_settop(L, -2); // pop action
		else lua_rawseti(L, 5, index++);
	}
	luaL_setn(L, 5, index -1); // length of actions
	while (index < key) {
		lua_pushnil(L);
		lua_rawseti(L, 5, index++);
	}
	lua_settop(L, 0LL);
	return 0;
}

void lsn_simsch_debug_render() {
	lua_State* L = cLWrapper::Instance().m_luaVM;
	if (!L)
		return;

	char line[512];
	wchar wline[512];

	CFont::SetFontStyle(FONT_STANDARD);
	CFont::SetBackgroundOff();
	CFont::SetBackGroundOnlyTextOff();
	CFont::SetWrapx(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetScale(0.85f, 1.1f);
	CFont::SetCentreOff();
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH));
	CFont::SetJustifyOff();
	CFont::SetColor(CRGBA(240, 240, 240, 255));
	CFont::SetDropShadowPosition(1);
	CFont::SetPropOn();
	CFont::SetDropColor(CRGBA(0, 0, 0, 255));

	int32 savedTop = lua_gettop(L);

	// _G["simsch"]
	lua_pushstring(L, "simsch");
	lua_gettable(L, LUA_GLOBALSINDEX);
	if (!lua_istable(L, -1)) { lua_settop(L, savedTop); return; }

	// simsch["actions"]
	lua_pushstring(L, "actions");
	lua_gettable(L, -2);
	if (!lua_istable(L, -1)) { lua_settop(L, savedTop); return; }

	int actions_idx = lua_gettop(L); 
	float x = 24.0f;
	float y = 24.0f;
	float ystep = 20.0f;
	int32 maxLines = 40;
	int32 maxFieldsShown = 40;

	snprintf(line, sizeof(line), "[SIMSCH ACTIONS]");
	AsciiToUnicode(line, wline);
	CFont::SetColor(CRGBA(240, 20, 20, 255));
	CFont::PrintString(x, y, wline);
	y += ystep;
	CFont::SetColor(CRGBA(200, 200, 200, 255));

	int32 printed = 0;
	int32 total = 0;

	lua_pushnil(L);
	while (lua_next(L, actions_idx) != 0) {
		// stack: actions_table(at actions_idx), key(-2), value(action)(-1)
		++total;

		char keyBuf[128] = { 0 };
		int keyType = lua_type(L, -2);
		if (keyType == LUA_TSTRING) {
			const char* s = lua_tostring(L, -2);
			snprintf(keyBuf, sizeof(keyBuf), "key=\"%s\"", s ? s : "");
		}
		else if (keyType == LUA_TNUMBER) {
			double n = lua_tonumber(L, -2);
			snprintf(keyBuf, sizeof(keyBuf), "key=%g", n);
		}
		else if (keyType == LUA_TBOOLEAN) {
			int b = lua_toboolean(L, -2);
			snprintf(keyBuf, sizeof(keyBuf), "key=%s", b ? "true" : "false");
		}
		else {
			const char* tn = lua_typename(L, keyType);
			const void* p = lua_topointer(L, -2);
			snprintf(keyBuf, sizeof(keyBuf), "key=<%s %p>", tn ? tn : "unknown", p);
		}

		if (printed < maxLines) {
			int action_idx = lua_gettop(L);

			// --- dead ---
			char deadBuf[32] = { 0 };
			if (lua_istable(L, action_idx)) {
				lua_pushstring(L, "dead");
				lua_gettable(L, action_idx);
				if (!lua_isnil(L, -1)) {
					int deadFlag = lua_toboolean(L, -1);
					if (lua_type(L, -1) == LUA_TBOOLEAN) {
						snprintf(deadBuf, sizeof(deadBuf), "dead=%s", deadFlag ? "Y" : "N");
					}
					else if (lua_type(L, -1) == LUA_TNUMBER) {
						double d = lua_tonumber(L, -1);
						snprintf(deadBuf, sizeof(deadBuf), "dead=%g", d);
					}
					else {
						const char* t = lua_typename(L, lua_type(L, -1));
						const void* p = lua_topointer(L, -1);
						snprintf(deadBuf, sizeof(deadBuf), "dead=<%s %p>", t ? t : "?", p);
					}
				}
				lua_pop(L, 1); // pop dead
			}
			else {
				// value isn't table
				deadBuf[0] = '\0';
			}

			// --- state ---
			char stateBuf[128] = { 0 };
			if (lua_istable(L, action_idx)) {
				lua_pushstring(L, "state");
				lua_gettable(L, action_idx);
				if (!lua_isnil(L, -1)) {
					int stt = lua_type(L, -1);
					if (stt == LUA_TSTRING) {
						const char* s = lua_tostring(L, -1);
						snprintf(stateBuf, sizeof(stateBuf), "state=\"%s\"", s ? s : "");
					}
					else if (stt == LUA_TNUMBER) {
						double n = lua_tonumber(L, -1);
						snprintf(stateBuf, sizeof(stateBuf), "state=%g", n);
					}
					else if (stt == LUA_TBOOLEAN) {
						snprintf(stateBuf, sizeof(stateBuf), "state=%s", lua_toboolean(L, -1) ? "true" : "false");
					}
					else {
						const char* tn = lua_typename(L, stt);
						const void* p = lua_topointer(L, -1);
						snprintf(stateBuf, sizeof(stateBuf), "state=<%s %p>", tn ? tn : "unknown", p);
					}
				}
				lua_pop(L, 1);
			}

			// --- script/type info ---
			char scriptBuf[128] = { 0 };
			lua_pushstring(L, "script");
			lua_gettable(L, action_idx);
			if (!lua_isnil(L, -1)) {
				int st = lua_type(L, -1);
				if (st == LUA_TTHREAD) {
					lua_State* th = lua_tothread(L, -1);
					int th_top = th ? lua_gettop(th) : 0;
					snprintf(scriptBuf, sizeof(scriptBuf), "script=thread top=%d ptr=%p", th_top, (void*)th);
				}
				else if (st == LUA_TFUNCTION) {
					snprintf(scriptBuf, sizeof(scriptBuf), "script=%s function", lua_iscfunction(L, -1) ? "C" : "Lua");
				}
				else {
					const char* tn = lua_typename(L, st);
					const void* p = lua_topointer(L, -1);
					snprintf(scriptBuf, sizeof(scriptBuf), "script=<%s %p>", tn ? tn : "unknown", p);
				}
			}
			lua_pop(L, 1);

			char fieldsBuf[256] = { 0 };
			if (lua_istable(L, action_idx)) {
				int fcount = 0;
				lua_pushnil(L);
				while (fcount < maxFieldsShown && lua_next(L, action_idx) != 0) {
					// stack: ... action_table (at action_idx), k (at -2), v (at -1)
					char kbuf[64] = { 0 };
					int kt = lua_type(L, -2);
					if (kt == LUA_TSTRING) {
						const char* ks = lua_tostring(L, -2);
						snprintf(kbuf, sizeof(kbuf), "%s", ks ? ks : "");
					}
					else if (kt == LUA_TNUMBER) {
						double kn = lua_tonumber(L, -2);
						snprintf(kbuf, sizeof(kbuf), "%g", kn);
					}
					else {
						const char* tname = lua_typename(L, kt);
						const void* pp = lua_topointer(L, -2);
						snprintf(kbuf, sizeof(kbuf), "<%s %p>", tname ? tname : "?", pp);
					}

					char vbuf[128] = { 0 };
					int vt = lua_type(L, -1);
					if (vt == LUA_TSTRING) {
						const char* vs = lua_tostring(L, -1);
						snprintf(vbuf, sizeof(vbuf), "\"%s\"", vs ? vs : "");
					}
					else if (vt == LUA_TNUMBER) {
						double vn = lua_tonumber(L, -1);
						snprintf(vbuf, sizeof(vbuf), "%g", vn);
					}
					else if (vt == LUA_TBOOLEAN) {
						snprintf(vbuf, sizeof(vbuf), "%s", lua_toboolean(L, -1) ? "true" : "false");
					}
					else {
						const char* tn = lua_typename(L, vt);
						const void* pp = lua_topointer(L, -1);
						snprintf(vbuf, sizeof(vbuf), "<%s %p>", tn ? tn : "unknown", pp);
					}

					// append "k=v, " to fieldsBuf (safe)
					char frag[256];
					snprintf(frag, sizeof(frag), "%s=%s", kbuf, vbuf);
					if (fieldsBuf[0]) strncat(fieldsBuf, ", ", sizeof(fieldsBuf) - strlen(fieldsBuf) - 1);
					strncat(fieldsBuf, frag, sizeof(fieldsBuf) - strlen(fieldsBuf) - 1);

					lua_pop(L, 1);
					++fcount;
				}
				if (lua_type(L, -1) != LUA_TTABLE && lua_type(L, -1) != LUA_TNIL && lua_gettop(L) > action_idx) {
					lua_pop(L, 1);
				}
			}

			snprintf(line, sizeof(line), "#%03d  %s  %s %s %s",
				total,
				keyBuf,
				stateBuf[0] ? stateBuf : "(no state)",
				deadBuf[0] ? deadBuf : "",
				scriptBuf[0] ? scriptBuf : ""
			);

			AsciiToUnicode(line, wline);
			CFont::PrintString(x, y, wline);
			y += ystep;
			++printed;

			if (fieldsBuf[0]) {
				snprintf(line, sizeof(line), "    fields: %s", fieldsBuf);
				AsciiToUnicode(line, wline);
				CFont::PrintString(x + 8.0f, y, wline);
				y += ystep;
			}
		}
		lua_pop(L, 1);
	}

	if (total > maxLines) {
		snprintf(line, sizeof(line), "... and %d more actions", total - maxLines);
		AsciiToUnicode(line, wline);
		CFont::PrintString(x, y, wline);
		y += ystep;
	}

	//return;
	//// ====================== ENTITY TRACK TABLE ======================
	//CFont::SetColor(CRGBA(20, 240, 20, 255));
	//snprintf(line, sizeof(line), "[ENTITY TRACK]  (weak table __mode=\"k\")");
	//AsciiToUnicode(line, wline);
	//CFont::PrintString(x, y, wline);
	//y += ystep;

	//CFont::SetColor(CRGBA(200, 200, 200, 255));

	//luaL_getmetatable(L, "EntityTrack");
	//if (lua_istable(L, -1)) {
	//	int entity_idx = lua_gettop(L);
	//	int32 et_total = 0;
	//	int32 et_printed = 0;

	//	lua_pushnil(L);
	//	while (lua_next(L, entity_idx) != 0 && et_printed < maxLines) {
	//		++et_total;

	//		if (lua_islightuserdata(L, -2)) {  // key = handle
	//			uint32 nHandle = (uint32)lua_touserdata(L, -2);

	//			uint8 owner = TRACKED_ENTITY_OWNER(nHandle);
	//			uint16 id = TRACKED_ENTITY_ID(nHandle);
	//			bool tracked = TRACKED_ENTITY_TRACK(nHandle);

	//			snprintf(line, sizeof(line), "0x%08X  owner=0x%02X  id=0x%04X  track=%d",
	//				nHandle, owner, id, tracked ? 1 : 0);

	//			AsciiToUnicode(line, wline);
	//			CFont::PrintString(x, y, wline);
	//			y += ystep;
	//			++et_printed;
	//		}

	//		lua_pop(L, 1);  // pop value
	//	}

	//	if (et_total > maxLines) {
	//		snprintf(line, sizeof(line), "... and %d more tracked entities", et_total - maxLines);
	//		AsciiToUnicode(line, wline);
	//		CFont::PrintString(x, y, wline);
	//		y += ystep;
	//	}
	//}
	//else {
	//	snprintf(line, sizeof(line), "(EntityTrack not found or not a table)");
	//	AsciiToUnicode(line, wline);
	//	CFont::PrintString(x, y, wline);
	//	y += ystep;
	//}
	//lua_pop(L, 1);  // pop EntityTrack metatable

	lua_settop(L, savedTop);
}

void lscript_open_simsch() {
	lua_State* L = cLWrapper::Instance().m_luaVM;
	lua_newtable(L);
	lua_pushstring(L, "start");
	lua_pushcclosure(L, lsn_simsch_start, 0);
	lua_settable(L, -3);
	lua_pushstring(L, "stop");
	lua_pushcclosure(L, lsn_simsch_stop, 0);
	lua_settable(L, -3);
	lua_pushstring(L, "step");
	lua_pushcclosure(L, lsn_simsch_step, 0);
	lua_settable(L, -3);
	lua_pushstring(L, "actions");
	lua_newtable(L);
	lua_settable(L, -3);
	lua_pushstring(L, "simsch");
	lua_insert(L, -2);
	lua_settable(L, LUA_GLOBALSINDEX);
}