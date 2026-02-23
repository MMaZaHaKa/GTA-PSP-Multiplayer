/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "Camera.h"


#include "multiplayer/natives/public.h"
#include "multiplayer/events/public.h"
#include "multiplayer/net/packet.h"
#include "multiplayer/MultiGame.h"


int mp_lsn_CameraFadeIn(lua_State* L) {
	float fTimeout = luaL_checknumber(L, 1);
	if (lua_gettop(L) >= 2) {
		CRGBA color = CRGBA_UNPACK(lsc_getColor(L, 2));
		TheCamera.SetFadeColour(color.red, color.green, color.blue);
	}
	TheCamera.Fade(1, fTimeout);
	TheCamera.ProcessFade();
	return 0;
}

int mp_lsn_CameraFadeOut(lua_State* L) {
	float fTimeout = luaL_checknumber(L, 1);
	if (lua_gettop(L) >= 2) {
		CRGBA color = CRGBA_UNPACK(lsc_getColor(L, 2));
		TheCamera.SetFadeColour(color.red, color.green, color.blue);
	}
	TheCamera.Fade(0, fTimeout);
	TheCamera.ProcessFade();
	return 0;
}

int mp_lsn_IsCameraFading(lua_State* L) {
	// TODO: reLVCS missing implementation for CCamera::GetFadingStatusForScript
	//lua_pushboolean(L, TheCamera.GetFadingStatusForScript());
	TODO();
	lsn_none(L);
	return 1;
}

int mp_lsn_SetFixedCamera(lua_State* L) {
	int nParams = lua_gettop(L);
	CVector pos, target;
	lsc_getVectorFromStack(pos, L, 1, true);
	lsc_getVectorFromStack(target, L, 2, true);
	net::pckt_set_fixed_camera packet;
	packet.pckt_size = sizeof(net::pckt_set_fixed_camera);
	packet.pckt_id = gtMP_PacketIDs.set_fixed_camera.pckt_id;
	packet.pos = pos;
	packet.target = target;
	if (nParams >= 3) {
		int nDestPlayerId = lsc_getPlayer(L, 3);
		TheMPGame.SendMessagePriority(packet, nDestPlayerId);
		return 0;
	}
	on_recv_set_fixed_camera(packet, 0, 0, false); // bug? true from local game?
	return 0;
}

int mp_lsn_RestoreCamera(lua_State* L) {
	int nParams = lua_gettop(L);
	bool bJumpcut = lua_tonumber(L, 1) != 0;
	net::pckt_restore_camera packet;
	packet.pckt_size = sizeof(net::pckt_restore_camera);
	packet.pckt_id = gtMP_PacketIDs.restore_camera.pckt_id;
	packet.jumpcut = bJumpcut;
	//packet.jumpcut = (packet.jumpcut & 0xFE) | bJumpcut;
	if (nParams >= 2) {
		int nDestPlayerId = lsc_getPlayer(L, 2);
		TheMPGame.SendMessagePriority(packet, nDestPlayerId);
	}
	on_recv_restore_camera(packet, 0, 0, false); // bug? true?
#ifdef FIX_BUGS
	return 0;
#else
	return 1;
#endif
}

static const luaL_reg ls_camera_lib[] = {
	{ "CameraFadeIn",   mp_lsn_CameraFadeIn},
	{ "CameraFadeOut",  mp_lsn_CameraFadeOut},
	{ "IsCameraFading", mp_lsn_IsCameraFading},
	{ "SetFixedCamera", mp_lsn_SetFixedCamera},
	{ "RestoreCamera",  mp_lsn_RestoreCamera},
	{NULL, NULL}
};
VALIDATE_LUA_LIB(ls_camera_lib, (5 + 1), (5 + 1));


void lscript_open_camera() {
	cMultiGame& Game = TheMPGame;
	cLWrapper& wrapper = cLWrapper::Instance();
#define REGISTER_PACKET(id, callback) Game.RegisterPacket(id, new cPacketDispatcher(callback));

	REGISTER_PACKET(gtMP_PacketIDs.set_fixed_camera.pckt_id, &on_recv_set_fixed_camera); // 21 lcs vcs
	REGISTER_PACKET(gtMP_PacketIDs.restore_camera.pckt_id, &on_recv_restore_camera); // 22 lcs vcs
	wrapper.CreateGlobalLibrary(ls_camera_lib, nil);
#undef REGISTER_PACKET
}