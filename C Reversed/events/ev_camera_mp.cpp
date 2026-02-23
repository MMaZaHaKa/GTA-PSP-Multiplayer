/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/packet.h"

#include "common.h"
#include "Camera.h"


// OLD on_recv_set_camera_at_fixed_pos
void on_recv_set_fixed_camera(net::pckt_set_fixed_camera& packet, int sender, uint16 time, bool bFromRing) // ID 21
{
	CVector source(packet.pos.x, packet.pos.y, packet.pos.z);
	TheCamera.SetCamPositionForFixedMode(source, CVector(0, 0, 0));
	CVector target(packet.target.x, packet.target.y, packet.target.z);
	TheCamera.TakeControlNoEntity(target, INTERPOLATION, CAMCONTROL_SCRIPT);
}

void on_recv_restore_camera(net::pckt_restore_camera& packet, int sender, uint16 time, bool bFromRing) // ID 22
{
	if (packet.jumpcut)
		TheCamera.RestoreWithJumpCut();
	else
		TheCamera.Restore();
}
