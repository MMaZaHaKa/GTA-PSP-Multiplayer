/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"

#include "World.h"
#include "multiplayer/elements/sHalo.h"
#include "SpecialFX.h"
#include "Camera.h"


sHaloElement::sHaloElement(uint16 id, CVector& pos, CVector& ext, uint32 colour, float fMarkerSize)
{
	m_nHandleID = id;
	m_fMarkerSize = fMarkerSize;
	m_Colour = BGRA_UNPACK(colour /*& 0x00FFFFFF*/);
	m_vecExtent = ext;
	m_vecPos = pos;
	if (m_vecPos.z <= MAP_Z_LOW_LIMIT)
		m_vecPos.z = CWorld::FindGroundZForCoord(m_vecPos.x, m_vecPos.y);
}

void sHaloElement::Update()
{
	if (m_fMarkerSize > 0.0f) {
		C3dMarkers::PlaceMarker((uint32)this, MARKERTYPE_CYLINDER, m_vecPos, m_fMarkerSize * 0.7f,
			m_Colour.red, m_Colour.green, m_Colour.blue, 150, 128, 0.0f, 1, 100.0f, 0, nil);
	}
}


sHalo::sHalo()
{
	m_nCount = 0;
	m_HaloElements = std::vector<sHaloElement*>();
}

void sHalo::Init()
{
	;
}

uint16 sHalo::Add(CVector& pos, CVector& ext, uint32 colour, float fMarkerSize)
{
	m_nCount++;
	sHaloElement* pElem = new sHaloElement(m_nCount, pos, ext, colour, fMarkerSize);
	m_HaloElements.push_back(pElem);
	return m_nCount;
}

void sHalo::Remove(uint16 id)
{
	std::vector< sHaloElement*>::iterator it;
	for (it = m_HaloElements.begin(); it != m_HaloElements.end(); it++)
	{
		if ((*it)->m_nHandleID == id) {
			delete (*it);
			m_HaloElements.erase(it);
			break;
		}
	}
}

void sHalo::Reset() 
{
	for (auto elem : m_HaloElements) {
		delete elem;
	}
	m_HaloElements.clear();
#ifdef FIX_BUGS
	m_nCount = 0;
#endif
}

void sHalo::Update()
{
	std::vector< sHaloElement*>::iterator it;
	for (it = m_HaloElements.begin(); it != m_HaloElements.end(); it++) {
		(*it)->Update();
	}
}
