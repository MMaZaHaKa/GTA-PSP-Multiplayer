/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"
#include "multiplayer/elements/sElement.h"

enum ePlayerPressKey
{
	HORN         = BIT(0),
	EXIT_VEHICLE = BIT(1),
	LOOK_LEFT    = BIT(2),
	LOOK_RIGHT   = BIT(3),
};

enum ePlayerSync
{
	MP_PKTD_PLR_EQUAL = 0,
	MP_PKTD_PLR_POSX  = BIT(0), // vPos.x
	MP_PKTD_PLR_POSY  = BIT(1), // vPos.y
	MP_PKTD_PLR_POSZ  = BIT(2), // vPos.z
	MP_PKTD_PLR_KEYS  = BIT(3), // bfKeyPress
	MP_PKTD_PLR_CAMF  = BIT(4), // vCamFront
	MP_PKTD_PLR_CAMS  = BIT(5), // vCamSource
	MP_PKTD_PLR_CAMU  = BIT(6), // vCamUp
	MP_PKTD_PLR_CAMV  = BIT(7), // fCamFov
	MP_PKTD_PLR_PICK  = BIT(8), // m_nPickups
	MP_PKTD_PLR_STAT  = BIT(9), // eWBState
	MP_PKTD_PLR_FULL  = -1,     // full diff u16 0xFFFF
};


struct sPlayerSync : sElementSync {
public:
	//int8 m_nPad1[8];
	CVector m_vPos;
	uint8 m_nKeyPresses; // ePlayerPressKey
	//int8 m_nPad2[15];
	CVector m_vCamFront;
	CVector m_vCamSource;
	CVector m_vCamUp;
	float m_fCamFov;
	int16 m_eWBState;
	uint8 m_nPickups; // ePowerupType
	//int8 m_nPad3[9];

	sPlayerSync();
	// sElementSync.unk1 = 0; seems like sPlayerSync(const sPlayerSync& other), but avoid copy sElementSync OR! sPlayerSync(sPlayerSync other)
	sPlayerSync(CVector vPos, uint8 nKeyPresses, CVector vCamFront, CVector vCamSource, CVector vCamUp, uint8 nPickups, int8 eWBState, float fCamFov);
	sPlayerSync(const sPlayerSync& other); // inlined
	~sPlayerSync() override;

	bool Compare(const sPlayerSync& other);
#if !defined(FINAL) && !defined(MASTER)
	void Dump();
#endif

#ifdef MULTIGAME_ELEMENTS_IMPROVEMENTS
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_PLAYER; }
#endif
};

struct sPlayer : sElement {
private:
	int32 m_nBlipIndex;
	//int8 m_nPad0[12];
	CMGMatrix m_nMatrix; // seems CMatrix because bfOwner + pRwMatr
	float m_fCamCosHFOV;
	float m_fCamSinHFOV;
	float m_fCamCosHFOVAspect; // m_fCamCosHFOVAdj
	float m_fCamSinHFOVAspect; // m_fCamSinHFOVAdj
public:

	sPlayer();

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sPlayer::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sPlayer() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_PLAYER; }
	void ApplyClientSync(uint16 nTime) override;
	void Update(uint16 nTime) override;
	bool WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) override;
	void ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) override;
	void RegisterSelfWithOwner(uint8 owner, uint16 id) override;

	void OnPlayerRender() {}
	void OnPlayerDelete() {}
	CVector& GetPosition();
	bool isPressingHorn();
	bool isPressingExitVehicle();
	bool isPositionInRadius(const CVector& pos, float radius, float maxDist);
	bool IsEntityInRadius(CEntity* ent, float maxDist);
	sPlayer* GetLockOnTarget();
	int32 GetOwnerID() { return sElement::GetOwner(); }

	uint16 CompareSyncState(sPlayerSync* pSync, sPlayerSync* pLastSync);
	bool WriteSyncDelta(sWriteSyncStream* pSyncStream, sPlayerSync* pSync, sPlayerSync* pLastSync);
	void PerformWriteSync(sWriteSyncStream* pSyncStream, sPlayerSync* pSync, uint16 nDiffMask);

	inline int32 GetBlipIndex() { return m_nBlipIndex; }
	inline float GetCamCosHFOV() { return m_fCamCosHFOV; }
	inline float GetCamSinHFOV() { return m_fCamSinHFOV; }
	inline float GetCamCosHFOVAspect() { return m_fCamCosHFOVAspect; }
	inline float GetCamSinHFOVAspect() { return m_fCamSinHFOVAspect; }
};
