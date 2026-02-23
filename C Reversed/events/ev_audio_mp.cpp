/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/packet.h"

#include "common.h"
#include "AudioManager.h"


void on_recv_play_remote_sound(net::pckt_play_remote_sound& packet, int sender, uint16 time, bool bFromRing) // ID 15
{
	AudioManager.DirectlyEnqueueSample(packet.sample, packet.bank, packet.counter, packet.priority, packet.frequency, packet.volume, packet.frames, false);
}
