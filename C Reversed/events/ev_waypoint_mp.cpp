/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once
#include "multiplayer/net/packet.h"
#include "multiplayer/events/public.h"
#include "multiplayer/elements/sWaypoint.h"

#include "common.h"


void sWaypoint::OnSetWaypoint(net::pckt_set_waypoint& packet, int sender, uint16 time, bool bFromRing) // ID 52
{
	CVector vecPos = packet.vecPos;
	CVector vecLookAt = packet.vecLookAt;
	CVector vecHitSize = packet.vecHitSize;
	sWaypointElement* elem = new sWaypointElement(
		sender,
		packet.nWaypointID,
		vecPos,
		vecLookAt,
		vecHitSize,
		packet.colour,
		packet.bShowArrow,
		packet.fMarkerSize,
		packet.fArrowHeight,
		packet.nType);
	m_WaypointElements.push_back(elem);
}

void sWaypoint::OnClearWaypoint(net::pckt_clear_waypoint& packet, int sender, uint16 time, bool bFromRing) // ID 53
{
	Clear(sender, packet.nWaypointID);
}

void sWaypoint::OnHitWaypoint(net::pckt_hit_waypoint& packet, int sender, uint16 time, bool bFromRing) // ID 54
{
	RegisterHit(packet.nWaypointID);
}

