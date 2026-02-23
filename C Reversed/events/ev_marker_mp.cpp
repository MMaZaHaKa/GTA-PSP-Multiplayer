/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/packet.h"

#include "common.h"
#include "Radar.h"


void on_recv_add_3d_marker(net::pckt_add_3d_marker& packet, int sender, uint16 time, bool bFromRing) // ID 44
{
	TheRadar->AddMultiplayerMarker(packet.owner, packet.elem);
}

void on_recv_remove_3d_marker(net::pckt_remove_3d_marker& packet, int sender, uint16 time, bool bFromRing) // ID 45
{
	TheRadar->RemoveMultiplayerMarker(packet.owner, packet.elem);
}