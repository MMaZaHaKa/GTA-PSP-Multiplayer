/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"
#include "Ped.h"

//struct tModelInfoEntryMPPSP { // orig psp struct
//	const char* name;
//	const char* key;
//	int32 unlockLevel;
//	int32 gangID;
//	bool32 bHasSexyWalkAnim;
//};
struct tModelInfoEntryMP {
	char name[12];
	char key[8];
	int32 unlockLevel;
	int32 gangID;
	bool32 bHasSexyWalkAnim;
};

class cLobbyPed { // like CPlayerSkin
private:
	CPed* m_pPed;
	float m_fRotation;
	bool m_bReloadModel;
	// int8 pad1[3]; // pad?
	int m_nModelID;
	int m_nRequestedModelID;
	int m_nLoadedModelID;
	int m_nMPModelID;
public: // todo get set
	float m_fRequestTime;
private:
	float m_fWalkAnimTime;
	RwTexture* m_pHighlightTexture;
public:

	void SetModelIndex(int nModelID);
	void RequestModels(int nModelID);
	void Update();
	void Render();
	void Create();
	void Destroy();
	cLobbyPed();
	~cLobbyPed();
};

#ifdef GTA_LIBERTY
#define MAX_MP_MODELS (65)
#else
#define MAX_MP_MODELS (82)
#endif
extern tModelInfoEntryMP ga_netModelList[MAX_MP_MODELS];
