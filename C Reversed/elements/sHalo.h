/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"

#include <vector>

struct sHaloElement {
public:
	uint16 m_nHandleID;
	//int8 m_pad1[14];
	CVector m_vecPos;
	CVector m_vecExtent;
	float m_fMarkerSize;
	CRGBA m_Colour;

	sHaloElement(uint16 id, CVector& pos, CVector& ext, uint32 colour, float fMarkerSize);
	void Update();
};

struct sHalo {
private:
	uint16 m_nCount;
	//int8 m_pad1[2];
	std::vector<sHaloElement*> m_HaloElements;
public:

	sHalo();
	uint16 Add(CVector& pos, CVector& ext, uint32 colour, float fMarkerSize);
	void Remove(uint16 id);
	void Reset();
	void Update();

	static void Init();
};