/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "World.h"

#include "multiplayer/MultiGame.h" // +sWaypoint.h
#include "multiplayer/events/public.h"
#include "SpecialFX.h"

#ifdef GTA_LIBERTY
void prepare_waypoint_packet(net::pckt_set_waypoint& packet, uint16 id, CVector& pos, CVector& vecLookAt, CVector& hitSize, CRGBA colour, bool bShowArrow, float fMarkerSize, float fArrowHeight)
#else
void prepare_waypoint_packet(net::pckt_set_waypoint& packet, uint16 id, CVector& pos, CVector& vecLookAt, CVector& hitSize, CRGBA colour, bool bShowArrow, int nType, float fMarkerSize, float fArrowHeight)
#endif
{
	packet.pckt_size = sizeof(net::pckt_set_waypoint);
	packet.pckt_id = gtMP_PacketIDs.set_waypoint.pckt_id;
	packet.nWaypointID = id;
	packet.vecPos = pos;
	packet.vecLookAt = vecLookAt; // direction
	packet.vecHitSize = hitSize;
	packet.fMarkerSize = fMarkerSize;
	packet.colour = colour;
	packet.fArrowHeight = fArrowHeight;
	packet.bShowArrow = bShowArrow;
#ifndef GTA_LIBERTY
	packet.nType = nType;
#endif
}

sWaypointElement::sWaypointElement(int32 owner, uint16 id, CVector& pos, CVector& direction, CVector& hitSize, CRGBA colour, bool bShowArrow, float fMarkerSize, float fArrowHeight, int32 nType) {
	cMultiGame& Game = cMultiGame::Instance();
	m_nOwnerID = owner;
	m_nID = id;
	m_bWasHit = false;
	m_vecPos = pos;
	m_vecLookAt = direction;
	m_vecHitSize = hitSize;
	m_fMarkerSize = fMarkerSize;
	m_Colour = colour;
	m_fArrowHeight = fArrowHeight;
	m_bShowArrow = bShowArrow;
	m_nType = nType;
	if (m_vecPos.z <= MAP_Z_LOW_LIMIT)
		m_vecPos.z = CWorld::FindGroundZForCoord(m_vecPos.x, m_vecPos.y);

	if (Game.m_WaypointManager.GetRaceArrowVisible())
		cNavArrow::SetTarget(false, m_vecPos.x, m_vecPos.y, m_vecPos.z);
	else
		cNavArrow::ClearTarget();
}

bool sWaypointElement::Update(sWaypoint* pWaypoint) {
	cMultiGame& Game = cMultiGame::Instance();
	sPlayer* pPlayer = Game.GetPlayer(MP_HOST_INDEX); // Local player
	sPed* pPed = (sPed*)Game.GetEntityForHandle(pPlayer->GetOwner(), eElementID::MG_ELEMENT_PLAYER_PED_ID); // GetPlayerPed()

	if (m_fMarkerSize > 0.0f)
	{
		switch (m_nType)
		{
			case WAYPOINT_LAND:
			{
				C3dMarkers::PlaceMarker(m_nID, MARKERTYPE_CYLINDER, m_vecPos, m_fMarkerSize * 0.7f, 
					m_Colour.red, m_Colour.green, m_Colour.blue, m_Colour.alpha, 128, 0.0f, 1, 100.0f, true, false, nil);
				break;
			}
			case WAYPOINT_AIR:
			{
				C3dMarkers::PlaceMarker(m_nID, MARKERTYPE_RACE_RING, m_vecPos, 6.0f,
					m_Colour.red, m_Colour.green, m_Colour.blue, 100, 1, 1.0f, 0, 0.0f, false, false, &m_vecLookAt);
				break;
			}
			default:
			{
				assert(false && "todo enum");
				break;
			}
		}

		if (m_vecPos != m_vecLookAt && m_bShowArrow) {
			CVector pos = m_vecPos;
			pos.z = m_fArrowHeight;
			C3dMarkers::PlaceMarker(m_nID + 2, MARKERTYPE_RACE_ARROW, pos, 3.2f,
				m_Colour.red, m_Colour.green, m_Colour.blue, m_Colour.alpha, 1, 1.0f, 1, 0.0f, true, false, &m_vecLookAt);
		}
	}

	// Check hit (ignoring height)
	CVector pedPos = pPed->GetSync().ped->GetMatrix().GetPosition();
	CVector dir = (pedPos - m_vecPos);
	if (fabsf(dir.x) < m_vecHitSize.x && fabsf(dir.y) < m_vecHitSize.y) {
		if (m_nOwnerID == Game.LocalPlayerID()) {
			pWaypoint->RegisterHit(m_nID);
		}
		else
		{
			net::pckt_hit_waypoint packet{};
			packet.pckt_size = sizeof(net::pckt_hit_waypoint);
			packet.pckt_id = gtMP_PacketIDs.hit_waypoint.pckt_id;
			packet.nWaypointID = m_nID;
			Game.SendMessagePriority(packet, m_nOwnerID);
		}
		return true; // Remove
	}
	return false;
};


sWaypoint::sWaypoint() {
	m_nCount = 0;
	m_RbWaypointTree = std::map<uint16, std::pair<int32, bool>>();
	m_WaypointElements = std::vector<sWaypointElement*>();
	m_bRaceArrowVisible = true;
}

uint16 sWaypoint::AddEntry(int32 nPlayerID, CVector& pos, CVector& direction, CVector& hitSize, CRGBA colour, bool bShowArrow, int32 nType, float fMarkerSize, float fArrowHeight)
{
	cMultiGame& Game = cMultiGame::Instance();
	uint16 nWaypointID = m_nCount++;

	net::pckt_set_waypoint packet{};
#ifdef GTA_LIBERTY
	prepare_waypoint_packet(packet, nWaypointID, pos, direction, hitSize, colour, bShowArrow, fMarkerSize, fArrowHeight);
#else
	prepare_waypoint_packet(packet, nWaypointID, pos, direction, hitSize, colour, bShowArrow, nType, fMarkerSize, fArrowHeight);
#endif

	if (nPlayerID == Game.LocalPlayerID())
		OnSetWaypoint(packet, nPlayerID, 0, true);
	else
		Game.SendMessagePriority(packet, nPlayerID);

	return nWaypointID;
}

void sWaypoint::Remove(uint16 nWaypointID) {
	auto it = m_RbWaypointTree.find(nWaypointID);
	if (it != m_RbWaypointTree.end()) {
		int32 owner = it->second.first;
		cMultiGame& Game = cMultiGame::Instance();
		if (owner == Game.LocalPlayerID()) {
			Clear(owner, nWaypointID);
		}
		else {
			net::pckt_clear_waypoint packet{};
			packet.pckt_size = sizeof(net::pckt_clear_waypoint);
			packet.pckt_id = gtMP_PacketIDs.clear_waypoint.pckt_id;
			packet.nWaypointID = nWaypointID;
			Game.SendMessagePriority(packet, owner);
		}
		m_RbWaypointTree.erase(it);
	}
}

void sWaypoint::RegisterHit(int32 nWaypointID)
{
	auto it = m_RbWaypointTree.find(nWaypointID);
	if (it != m_RbWaypointTree.end()) {
		it->second.second = true;
	}
}

void sWaypoint::Update()
{
	for (auto it = m_WaypointElements.begin(); it != m_WaypointElements.end(); ) {
		if ((*it)->Update(this)) {
			delete* it;
			it = m_WaypointElements.erase(it);
		}
		else {
			++it;
		}
	}
}

void sWaypoint::Reset() {
	for (auto elem : m_WaypointElements) {
		delete elem;
	}
	m_WaypointElements.clear();
	m_RbWaypointTree.clear();
	m_nCount = 0;
}

void sWaypoint::Clear(int32 nOwnerID, int32 nWaypointID)
{
	for (auto it = m_WaypointElements.begin(); it != m_WaypointElements.end(); ++it) {
		if ((*it)->m_nOwnerID == nOwnerID && (*it)->m_nID == nWaypointID) {
			delete* it;
			m_WaypointElements.erase(it);
			return;
		}
	}
}
