/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"

#include <vector>
#include <map>

enum eWaypointType
{
	WAYPOINT_LAND = 0,
	WAYPOINT_AIR,
};

struct sWaypoint;
struct sWaypointElement {
public:
	uint16 m_nOwnerID;
	uint16 m_nID;
	bool m_bWasHit;
	//int8 m_pad1[3];
	uint32 field_8;
	uint32 field_C;
	CVector m_vecPos;
	CVector m_vecLookAt;
	CVector m_vecHitSize;
	float m_fMarkerSize;
	CRGBA m_Colour;
	float m_fArrowHeight;
	bool m_bShowArrow;
	//int8 m_pad2[3];
	uint32 m_nType;

public:

	sWaypointElement(int32 owner, uint16 id, CVector& pos, CVector& direction, CVector& hitSize, CRGBA colour, bool bShowArrow, float fMarkerSize, float fArrowHeight, int32 nType);
	bool Update(sWaypoint* pWaypoint);

};

struct sWaypoint {
public:
	uint16 m_nCount;
	std::map<uint16, std::pair<int32, bool>> m_RbWaypointTree; // id -> (owner, hit)
	//int8 m_pad1[2];
	std::vector<sWaypointElement*> m_WaypointElements;
	bool m_bRaceArrowVisible;
public:

	sWaypoint();
	//sWaypoint() : m_nCount(0), m_bRaceArrowVisible(true) {}
	uint16 AddEntry(int32 nPlayerID, CVector& pos, CVector& direction, CVector& hitSize, CRGBA colour, bool bShowArrow, int32 nType, float fMarkerSize, float fArrowHeight);
	void Remove(uint16 nWaypointID);
	void RegisterHit(int32 nWaypointID);
	void Update();
	void SetRaceArrowVisible(bool visible) { m_bRaceArrowVisible = visible; }
	inline bool GetRaceArrowVisible() { return m_bRaceArrowVisible; }
	void Reset();
	void Clear(int32 nOwnerID, int32 nWaypointID);

	void OnSetWaypoint(net::pckt_set_waypoint& packet, int sender, uint16 time, bool fromLocalGame); // ID 52
	void OnClearWaypoint(net::pckt_clear_waypoint& packet, int sender, uint16 time, bool fromLocalGame); // ID 53
	void OnHitWaypoint(net::pckt_hit_waypoint& packet, int sender, uint16 time, bool fromLocalGame); // ID 54
};

#ifdef GTA_LIBERTY
void prepare_waypoint_packet(net::pckt_set_waypoint& packet, uint16 id, CVector& pos, CVector& vecLookAt, CVector& hitSize, CRGBA colour, bool bShowArrow, float fMarkerSize, float fArrowHeight);
#else
void prepare_waypoint_packet(net::pckt_set_waypoint& packet, uint16 id, CVector& pos, CVector& vecLookAt, CVector& hitSize, CRGBA colour, bool bShowArrow, int nType, float fMarkerSize, float fArrowHeight);
#endif