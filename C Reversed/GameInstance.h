/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "common.h"

extern uint8 GAME_ID;
void InitInstanceCounter();
void ShutdownInstanceCounter();
int GetInstancePeerCount();
inline bool IsFirstInstance() { return GAME_ID == 1; }
