/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "Text.h"
#include "Messages.h"
#include "Hud.h"

#include "leeds/base/stringt.h"

#include "multiplayer/natives/public.h"
#include "multiplayer/events/public.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/MultiGame.h"

#include "multiplayer/elements/sTextSprite.h"


sTextSprite* lsc_get_text_sprite(lua_State* L) {
	cMultiGame& pGame = cMultiGame::Instance();
	void* pData = luaL_checkudata(L, 1, "textsprite");
	if (!pData) return nil;
	int32 nHandle = *((int*)pData);
	if (nHandle == -1) return nil;
	return (sTextSprite*)pGame.GetEntityForHandle(pGame.LocalPlayerID(), nHandle);
}

int mp_lsn_Flash(lua_State* L) {
	int32 nFlash = 0;
	sTextSprite* pSprite = lsc_get_text_sprite(L);
	if (!pSprite) return 0;
	if (lua_gettop(L) >= 2) {
		nFlash = luaL_checknumber(L, 2);
		pSprite->SetFlash(nFlash);
		return 0;
	}
	nFlash = pSprite->GetFlash();
	lua_pushnumber(L, nFlash);
	return 1;
}

int mp_lsn_Order(lua_State* L) {
	float fOrder = 0;
	sTextSprite* pSprite = lsc_get_text_sprite(L);
	if (!pSprite) return 0;
	if (lua_gettop(L) >= 2) {
		fOrder = luaL_checknumber(L, 2);
		pSprite->SetOrder(fOrder);
		return 0;
	}
	fOrder = pSprite->GetOrder();
	lua_pushnumber(L, fOrder);
	return 1;
}

int mp_lsn_Scale(lua_State* L) {
	float fScaleX = 0, fScaleY = 0;
	sTextSprite* pSprite = lsc_get_text_sprite(L);
	if (!pSprite) return 0;
	if (lua_gettop(L) >= 2) {
		fScaleX = lua_tonumber(L, 2);
		fScaleY = lua_tonumber(L, 3);
#ifdef GTA_LIBERTY
		fScaleY = fScaleY == 0 ? fScaleX * (2.173913) : fScaleY; // 50 / 23 recheck nickname print32 in draw3dmarkers 2.17
#else
		fScaleY = fScaleY == 0 ? fScaleX : fScaleY;
#endif
		pSprite->SetScale(fScaleX, fScaleY);
		return 0;
	}
	CVector2D* pScale = pSprite->GetScale();
	lua_pushnumber(L, pScale->x);
	lua_pushnumber(L, pScale->y);
	return 2;
}

int mp_lsn_Style(lua_State* L) {
	uint8 nStyle = 0;
	sTextSprite* pSprite = lsc_get_text_sprite(L);
	if (!pSprite) return 0;
	if (lua_gettop(L) >= 2) {
		nStyle = luaL_checknumber(L, 2);
		pSprite->SetStyle(nStyle);
		return 0;
	}
	nStyle = pSprite->GetStyle();
	lua_pushnumber(L, nStyle);
	return 1;
}

int mp_lsn_sColour(lua_State* L) {
	sTextSprite* pSprite = lsc_get_text_sprite(L);
	if (!pSprite) return 0;
	int32 nTop = lua_gettop(L);
	if (nTop < 2) {
		CRGBA* pColour = pSprite->GetColour();
		lua_pushnumber(L, CRGBA_PACK(pColour->r, pColour->g, pColour->b, pColour->a));
		return 1;
	}
	else if (nTop >= 3) {
		uint8 r = luaL_checknumber(L, 2);
		uint8 g = luaL_checknumber(L, 3);
		uint8 b = luaL_checknumber(L, 4);
		uint8 a = luaL_checknumber(L, 5);
		pSprite->SetColour(CRGBA(r, g, b, a));
		return 0;
	}
	CRGBA color = CRGBA_UNPACK_LEGACY(lsc_getColor(L, 2));
	pSprite->SetColour(color);
	return 0;
}

// old
///* stub */
//CRGBA* ls_get_entity_color(void *entity) {
//	return NULL;
//}
//
///* stub */
//CRGBA* ls_set_entity_color(void* entity, CRGBA* color) {
//	return NULL;
//}
//
//inline void unpack_color(CRGBA& dest, int32 src) {
//	dest.red = (src >> 24);
//	dest.green = (src >> 16);
//	dest.blue = (src >> 8);
//	dest.alpha = src;
//}
//
///* TODO: this function seems not to be from this file (Text Sprite colour) */
//int32 mp_lsn_Colour(lua_State* L) {
//	void* pUnk = mp_ls_getTextSpriteHandle(L);
//	if (!pUnk) return 0;
//	int32 nArgs = lua_gettop(L);
//	if (nArgs >= 3) {
//		CRGBA color(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5));
//		ls_set_entity_color(pUnk, &color);
//	}
//	else if (nArgs == 2) {
//		CRGBA color;
//		unpack_color(color, lsc_getColor(L, 2));
//		ls_set_entity_color(pUnk, &color);
//	}
//	// no args
//	CRGBA* pColor = ls_get_entity_color(pUnk);
//	uint32 color = CRGBA_PACK(pColor->red, pColor->green, pColor->blue, pColor->alpha);
//	lua_pushnumber(L, color);
//	return 1;
//}


int mp_lsn_WrapX(lua_State* L) {
	uint8 nWrapX = 0;
	sTextSprite* pSprite = lsc_get_text_sprite(L);
	if (!pSprite) return 0;
	if (lua_gettop(L) >= 2) {
		nWrapX = luaL_checknumber(L, 2);
		pSprite->SetWrapX(nWrapX);
		return 0;
	}
	nWrapX = pSprite->GetWrapX();
	lua_pushnumber(L, nWrapX);
	return 1;
}

int mp_lsn_Align(lua_State* L) {
	uint8 eAlign = 0;
	sTextSprite* pSprite = lsc_get_text_sprite(L);
	if (!pSprite) return 0;
	if (lua_gettop(L) >= 2) {
		eAlign = luaL_checknumber(L, 2);
		pSprite->SetAlign(eAlign);
		return 0;
	}
	eAlign = pSprite->GetAlign();
	lua_pushnumber(L, eAlign);
	return 1;
}

int mp_lsn_Text(lua_State* L) {
	sTextSprite* pSprite = lsc_get_text_sprite(L);
	if (!pSprite) return 0;
	if (lua_gettop(L) >= 2) {
		base::string sText = luaL_checkstring(L, 2);
		pSprite->SetText(sText);
		return 0;
	}
	base::string* pText = pSprite->GetText();
	lua_pushstring(L, pText->c_str());
	return 1;
}

int mp_lsn_Pos(lua_State* L) {
	sTextSprite* pSprite = lsc_get_text_sprite(L);
	if (!pSprite) return 0;
	if (lua_gettop(L) >= 2) {
		int32 nPosX = luaL_checknumber(L, 2);
		int32 nPosY = luaL_checknumber(L, 3);
		float fScale = lua_tonumber(L, 4);
		pSprite->SetPos(nPosX, nPosY, fScale);
		return 0;
	}
	CVector2D* pPos = pSprite->GetPos();
	lua_pushnumber(L, pPos->x);
	lua_pushnumber(L, pPos->y);
	return 2;
}

int mp_lsn_RemoveTextSprite(lua_State* L) {
	cMultiGame& Game = cMultiGame::Instance();
	int* pHandleID = (int*)luaL_checkudata(L, 1, "textsprite");
	if (!pHandleID || *pHandleID == -1) return 0;
	sTextSprite* pSprite = (sTextSprite*)Game.GetEntityForHandle(Game.LocalPlayerID(), *pHandleID);
	if (pSprite) delete pSprite;
	*pHandleID = -1;
	return 0;
}

static const luaL_reg ls_text_sprite_lib[] = {
	{"Flash",  mp_lsn_Flash},
	{"Order",  mp_lsn_Order},
	{"Scale",  mp_lsn_Scale},
	{"Style",  mp_lsn_Style},
	{"Colour", mp_lsn_sColour},
	{"WrapX",  mp_lsn_WrapX},
	{"Align",  mp_lsn_Align},
	{"Text",   mp_lsn_Text},
	{"Pos",    mp_lsn_Pos},
	{"Remove", mp_lsn_RemoveTextSprite},
	{"__gc",   mp_lsn_RemoveTextSprite},
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_text_sprite_lib, (11 + 1), (11 + 1));

int mp_lsn_PrintNow(lua_State* L) {
	cMultiGame& pGame = TheMPGame;
	net::pckt_print_now packet;
#ifdef GTA_LIBERTY
	int32 nPeerID = lsc_pop_peer_id_from_stack(L, 1, 0xB00B5);
	const char* sKey = luaL_checkstring(L, 1);
	const int32 nTime = luaL_checknumber(L, 2);
	const int32 nFlag = luaL_checknumber(L, 3);
#else
	int32 nPeerID = lua_tonumber(L, 1);
	const char* sKey = luaL_checkstring(L, 2);
	const int32 nTime = luaL_checknumber(L, 3);
	const int32 nFlag = luaL_checknumber(L, 4);
#endif
	packet.pckt_size = sizeof(net::pckt_base) + sizeof(net::pckt_print_now::time) + sizeof(net::pckt_print_now::flag) + (uint16)strlen(sKey) + 1; // +12 [+3 +4 +4 + 1]
	//packet.pckt_size = (uint16)strlen(sKey) + 11 + 1; // +12 [+3 +4 +4 + 1]
	static_assert((sizeof(net::pckt_print_now) - 256) == 12, "need upd pckt_size calc");
	packet.pckt_id = gtMP_PacketIDs.print_now.pckt_id;
	packet.time = nTime;
	packet.flag = nFlag;
	strcpy(packet.key, sKey);
	if (pGame.IsLocalPlayer(nPeerID))
		on_recv_print_now(packet, 0, 0, false); // bug? true?
	pGame.SendMessage(packet, nPeerID);
	return 0;
}

void push_text_sprite(lua_State* L, sTextSprite* pSprite) {
	int* pHandleID = (int*)lua_newuserdata(L, sizeof(int));
	*pHandleID = pSprite->GetID();
	luaL_getmetatable(L, "textsprite");
	lua_setmetatable(L, -2);
}

/* TODO: stub */
int mp_lsn_TextSprite(lua_State* L) {
#ifdef GTA_LIBERTY
	int32 nPeerID = lsc_pop_peer_id_from_stack(L, 1, -1);
	const float fPosX = luaL_checknumber(L, 1);
	const float fPosY = luaL_checknumber(L, 2);
	const uint8 eAlign = lua_tonumber(L, 3);
#else
	int32 nPeerID = lua_tonumber(L, 1);
	float fPosX = lua_tonumber(L, 2);
	float fPosY = lua_tonumber(L, 3);
	const uint8 eAlign = lua_tonumber(L, 4);
#endif
	sTextSprite* pElement = new sTextSprite(nPeerID, eAlign, fPosX, fPosY);
	push_text_sprite(L, pElement);
	return 1;
}

int mp_lsn_ClearMessages(lua_State* L) {
	TheHud->ClearBigMessagesExcept(-1, -1);
	return 0;
}

static const luaL_reg ls_text_sprites_lib[] = {
	{"PrintNow",      mp_lsn_PrintNow},
	{"TextSprite",    mp_lsn_TextSprite},
	{"ClearMessages", mp_lsn_ClearMessages},
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_text_sprites_lib, (3 + 1), (3 + 1));


void lscript_open_text_sprite() {
	cMultiGame& Game = cMultiGame::Instance();
	cLWrapper& wrapper = cLWrapper::Instance();
#define REGISTER_PACKET(id, callback) Game.RegisterPacket(id, new cPacketDispatcher(callback));

	wrapper.CreateLibrary(ls_text_sprite_lib, "textsprite");
	REGISTER_PACKET(gtMP_PacketIDs.print_now.pckt_id, &on_recv_print_now); // 51, lcs vcs
	wrapper.CreateGlobalLibrary(ls_text_sprites_lib, nil);
#undef REGISTER_PACKET
}