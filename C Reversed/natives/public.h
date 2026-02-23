/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "common.h"

#include "LScript.h"
#include "multiplayer/elements/sElement.h"

#define TRACKED_ENTITY_TYPE       (0xA8000000)
#define TRACKED_ENTITY_TYPE_MASK  (0xFF000000)
#define TRACKED_ENTITY_OWNER_MASK (0x00FF0000)
#define TRACKED_ENTITY_ID_MASK    (0x0000FFFF)

int lsn_none(lua_State* L);

void lsc_getVectorFromStack(CVector& vec, lua_State* L, int index, bool allowEntity);
void lsc_getVectorFromStackNoEntity(CVector& vec, lua_State* L, int index);
void lsc_pushVuVector(lua_State* L, CVector& vec);
bool lsc_isPlayerUserData(lua_State* L, int index);
int32 lsc_getPlayer(lua_State* L, int index);
int32 lsc_getPlayerSafety(lua_State* L, int index);
bool lsc_is_entity_tracked(lua_State* L, int index);
uint32 lsc_get_tracked_entity(lua_State* L, int index);
void mp_lsc_transfer_entity(int nKeyOwner, int nKeyID, int nOwner, int nID);
void lsc_register_entity(lua_State* L, sElement *pElem);
sElement* lsc_get_entity(lua_State* L, int index);
uint32 lsc_getColor(lua_State* L, int index);
#ifdef GTA_LIBERTY
int lsc_pop_peer_id_from_stack(lua_State* L, int index, int unk);
#endif
int lsc_call(lua_State* L, int narg, bool clear);
void lsc_update_simsch();
void lsn_push_player_id(lua_State* L, int id);
#ifndef GTA_LIBERTY
void lsc_tryCreateMpPlayer();
#endif
int mp_lsn_PedPosition(lua_State* L); // for player.cpp

inline CRGBA lsc_getColour(lua_State* L, int32 nIndex) {
	return CRGBA(lua_tonumber(L, nIndex), lua_tonumber(L, nIndex + 1), lua_tonumber(L, nIndex + 2), lua_tonumber(L, nIndex + 3));
}

inline bool is_local_player(int id) {
	return id == -1 || id == cMultiGame::Instance().LocalPlayerID();
}

//inline bool is_peer_host(int id) {
//	return id == -1 || id == cMultiGame::Instance().LocalPlayerID();
//}

inline int get_player_id(lua_State* L) {
	int id = lsc_isPlayerUserData(L, 1) ? lsc_getPlayer(L, 1) : cMultiGame::Instance().LocalPlayerID();
	return id;
}

//inline void CVectorToRwV3d(RwV3d& dest, CVector& src) {
//	dest.x = src.x;
//	dest.y = src.y;
//	dest.z = src.z;
//}
//inline void RwV3dToCVector(RwV3d& src, CVector& dest) {
//	dest.x = src.x;
//	dest.y = src.y;
//	dest.z = src.z;
//}

inline void max_swap(float& a, float& b) {
	if (a > b) {
		float tmp = b;
		b = a;
		a = tmp;
	}
}

#define CRGBA_UNPACK_LEGACY(val) \
    CRGBA( (uint8)(((((uint32)(val)) << 8) + 255u) >> 24), \
           (uint8)(((((uint32)(val)) << 8) + 255u) >> 16), \
           (uint8)(((((uint32)(val) & 0xFFFFu) << 8) + 255u) >> 8), \
           (uint8)255 )


//int inline lsc_pack_color(uint8 red, uint8 green, uint8 blue, uint8 alpha) {
//	return CRGBA_PACK(red, green, blue, alpha);
//	//return (red << 24) + (green << 16) + (blue << 8) + alpha;
//}
//
//CRGBA inline lsc_unpack_color(uint32 color) {
//	return CRGBA_UNPACK(color);
//	//return CRGBA(color >> 24, color >> 16, color >> 8, color);
//}
//
//int inline lsc_pack_color(CRGBA& color) {
//	return CRGBA_PACK(color.red, color.green, color.blue, color.alpha);
//	//return lsc_pack_color(color.red, color.green, color.blue, color.alpha);
//}
//
//static uint32 inline pack_color(CRGBA& color) {
//	return (color.red << 16) + (color.green << 8) + color.blue;
//}
//
//static uint32 inline pack_color_with_alpha(CRGBA& color) {
//	return lsc_pack_color(color.red, color.green, color.blue, color.alpha);
//}

void lscript_open_ped();
void lscript_open_player();
void lscript_open_vehicle();
void lscript_open_radar();
void lscript_open_text_sprite();
#ifndef GTA_LIBERTY
void lscript_open_net2dMeter();
#endif
void lscript_open_waypoints();
void lscript_open_halo();
void lscript_open_pickups();
void lscript_open_garages();
void lscript_open_camera();
void lscript_open_roads();
void lscript_open_matching();
void lscript_open_cheats();
void lscript_open_constants();
void lscript_open_main();
void lscript_open_simsch();
