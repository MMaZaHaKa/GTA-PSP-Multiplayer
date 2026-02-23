/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "LobbyPed.h"
#include "TxdStore.h"
#include "Draw.h"
#include "Streaming.h"
#include "Timer.h"
#include "ModelIndices.h"
#include "Sprite.h"
#include "main.h"
#include "RpAnimBlend.h"
#include "AnimBlendAssociation.h"
#include "Lights.h"
#include "Camera.h"
#include "PlayerInfo.h"
#include "PlayerPed.h"
#include "MultiGame.h"

#define GET_CURRENT_TIME() (CTimer::GetTimeInMillisecondsPauseMode()/1000.0f)

// TODO: find a place to define this
tModelInfoEntryMP ga_netModelList[MAX_MP_MODELS] = {
#ifdef GTA_LIBERTY
	{"plr",         "MPC_000", 0, 11, false},
	{"plr2",        "MPC_001", 3, 11, false},
	{"plr3",        "MPC_002", 0, 11, false},
	{"plr4",        "MPC_003", 1, 11, false},
	{"plr5",        "MPC_004", 1, 11, false},
	{"plr6",        "MPC_005", 3, 11, false},
	{"plr7",        "MPC_006", 2, 11, false},
	{"plr8",        "MPC_007", 3, 11, false},
	{"plr9",        "MPC_008", 1, 11, false},
	{"plr10",       "MPC_009", 1, 11, false},
	{"plr11",       "MPC_010", 3, 11, false},
	{"plr12",       "MPC_011", 2, 11, false},
	{"plr13",       "MPC_060", 2, 11, false},
	{"plr14",       "MPC_061", 2, 11, false},
	{"plr15",       "MPC_063", 2, 11, false},
	{"plr16",       "MPC_062", 3, 11, false},
	{"tool_01",     "MPC_012", 1, 11, false},
	{"gimp",        "MPC_013", 1, 11, false},
	{"avery",       "MPC_014", 3, 11, false},
	{"baby",        "MPC_015", 1, 11, false},
	{"eight",       "MPC_016", 3, 11, false},
	{"mar_01",      "MPC_017", 1, 11, true},
	{"mickeyh",     "MPC_018", 1, 11, false},
	{"biker2",      "MPC_019", 2, 11, false},
	{"thug_01",     "MPC_020", 3, 7,  false},
	{"hitman",      "MPC_021", 2, 11, false},
	{"gang02",      "MPC_022", 0, 2,  false},
	{"gang03",      "MPC_023", 1, 3,  false},
	{"gang06",      "MPC_024", 1, 4,  false},
	{"gang07",      "MPC_025", 2, 5,  false},
	{"gang10",      "MPC_026", 2, 6,  false},
	{"gang11",      "MPC_027", 3, 9,  false},
	{"gang13",      "MPC_028", 3, 8,  false},
	{"gang16",      "MPC_029", 0, 1,  false},
	{"gang17",      "MPC_030", 0, 0,  false},
	{"cop",         "MPC_031", 0, 11, false},
	{"swat",        "MPC_032", 2, 11, false},
	{"fbi",         "MPC_033", 2, 11, false},
	{"army",        "MPC_034", 4, 11, false},
	{"fireman",     "MPC_035", 2, 11, false},
	{"pimp",        "MPC_036", 0, 11, false},
	{"prostitute",  "MPC_037", 4, 11, true},
	{"const1",      "MPC_038", 4, 11, false},
	{"vinc_01",     "MPC_039", 1, 11, false},
	{"sal_01",      "MPC_040", 3, 11, false},
	{"kazuki",      "MPC_064", 3, 11, false},
	{"wkas_01",     "MPC_041", 3, 11, true},
	{"kas_01",      "MPC_042", 3, 11, true},
	{"Fatfemale01", "MPC_043", 4, 11, true},
	{"biker1",      "MPC_044", 2, 11, false},
	{"gang14",      "MPC_045", 4, 11, false},
	{"ct_wom1",     "MPC_046", 0, 11, true},
	{"ct_man2",     "MPC_047", 4, 11, false},
	{"wayne",       "MPC_048", 1, 11, false},
	{"paulie",      "MPC_049", 3, 11, false},
	{"female01",    "MPC_050", 4, 11, true},
	{"male03",      "MPC_051", 4, 11, false},
	{"scum_man",    "MPC_052", 4, 11, false},
	{"taxi_d",      "MPC_053", 4, 11, false},
	{"deliass",     "MPC_054", 1, 11, false},
	{"criminal02",  "MPC_055", 4, 11, false},
	{"male01",      "MPC_056", 4, 11, false},
	{"p_wom1",      "MPC_057", 4, 11, true},
	{"B_wom1",      "MPC_058", 2, 11, true},
	{"prostitute2", "MPC_059", 0, 11, true},
#else
	{"plr",         "MPC_000", 0, 11, false},
	{"plr2",        "MPC_001", 0, 11, false},
	{"plr3",        "MPC_002", 1, 11, false},
	{"plr4",        "MPC_003", 1, 11, false},
	{"plr9",        "MPC_008", 1, 11, false},
	{"plr10",       "MPC_009", 1, 11, false},
	{"plr5",        "MPC_004", 2, 11, false},
	{"plr6",        "MPC_005", 2, 11, false},
	{"plr7",        "MPC_006", 3, 11, false},
	{"plr8",        "MPC_007", 3, 11, false},
	{"plr11",       "MPC_010", 3, 11, false},
	{"plr12",       "MPC_011", 3, 11, false},
	{"plr14",       "MPC_013", 3, 11, false},
	{"Jerry",       "MPC_141", 0, 11, false},
	{"Phil",        "MPC_153", 0, 11, false},
	{"Sarge",       "MPC_157", 0, 11, false},
	{"Marty",       "MPC_148", 1, 11, false},
	{"Alberto",     "MPC_126", 2, 11, false},
	{"Bry",         "MPC_130", 2, 11, false},
	{"LanSt",       "MPC_143", 2, 11, false},
	{"GayBiA",      "MPC_150", 2, 5,  false},
	{"Robber",      "MPC_156", 2, 11, false},
	{"Umber",       "MPC_159", 2, 11, false},
	{"Arman",       "MPC_127", 3, 11, false},
	{"Barry",       "MPC_128", 3, 11, false},
	{"CokHed",      "MPC_133", 3, 11, false}, // lol
	{"Diaz",        "MPC_134", 3, 11, false},
	{"Diego",       "MPC_135", 3, 11, false},
	{"Gonz",        "MPC_138", 3, 11, false},
	{"Hitman",      "MPC_139", 3, 11, false},
	{"LanSm",       "MPC_142", 3, 11, false},
	{"PhCol",       "MPC_152", 3, 11, false},
	{"Reni",        "MPC_154", 3, 11, false},
	{"RenOp",       "MPC_155", 3, 11, false},
	{"ZomA",        "MPC_160", 3, 11, false},
	{"Gang1A",      "MPC_097", 0, 0,  false},
	{"Gang2A",      "MPC_099", 0, 1,  false},
	{"Gang4A",      "MPC_103", 0, 11, false},
	{"Gang4B",      "MPC_104", 0, 11, false},
	{"Gang7A",      "MPC_109", 0, 2,  false},
	{"Gang5A",      "MPC_105", 1, 11, false},
	{"Gang5B",      "MPC_106", 1, 11, false},
	{"PGang7A",     "MPC_095", 2, 3,  false}, // pgang?
	{"Gang3A",      "MPC_101", 2, 9,  false},
	{"Gang6A",      "MPC_107", 2, 8,  false},
	{"Gang9A",      "MPC_113", 3, 4,  false},
	{"DiaGgA",      "MPC_115", 3, 6,  false},
	{"GonGgA",      "MPC_117", 3, 11, false},
	{"GonGgB",      "MPC_118", 3, 11, false},
	{"MenGgA",      "MPC_119", 3, 7,  false},
	{"cop",         "MPC_014", 0, 11, false},
	{"fireman",     "MPC_019", 0, 11, false},
	{"BMYPI",       "MPC_059", 0, 11, false},
	{"swat",        "MPC_015", 1, 11, false},
	{"fbi",         "MPC_016", 1, 11, false},
	{"WFYPR",       "MPC_082", 1, 11, true},
	{"vice1",       "MPC_123", 1, 11, false},
	{"BCop",        "MPC_125", 1, 11, false},
	{"HFOTR",       "MPC_038", 2, 11, true},
	{"army",        "MPC_017", 4, 11, false},
	{"HFYBE",       "MPC_030", 4, 11, true},
	{"HMYBE",       "MPC_032", 4, 11, false},
	{"HFYMD",       "MPC_035", 4, 11, true},
	{"HFYPR",       "MPC_037", 4, 11, true},
	{"HMOTR",       "MPC_039", 4, 11, false},
	{"BMYCR",       "MPC_043", 4, 11, false},
	{"BMYST",       "MPC_046", 4, 11, false},
	{"BMOTR",       "MPC_058", 4, 11, false},
	{"BMYBB",       "MPC_060", 4, 11, false},
	{"WMYCR",       "MPC_061", 4, 11, false},
	{"WFYST",       "MPC_062", 4, 11, true},
	{"WFYRI",       "MPC_066", 4, 11, true},
	{"WMYRI",       "MPC_068", 4, 11, false},
	{"WFYBE",       "MPC_070", 4, 11, true},
	{"WMYBE",       "MPC_071", 4, 11, false},
	{"WMOBE",       "MPC_073", 4, 11, false},
	{"WMYBU",       "MPC_080", 4, 11, false},
	{"WMYPI",       "MPC_085", 4, 11, false},
	{"WFYJG",       "MPC_087", 4, 11, true},
	{"WMYJG",       "MPC_088", 4, 11, false},
	{"DEA1",        "MPC_121", 4, 11, false},
	{"vice2",       "MPC_124", 4, 11, false},
#endif
};

void cLobbyPed::SetModelIndex(int nModelID) {
	if (!m_pPed)
		return;

	m_nRequestedModelID = nModelID;
	m_fRequestTime = GET_CURRENT_TIME();
}

void cLobbyPed::RequestModels(int nModelID) {
	if (!m_pPed)
		return;

	cMultiGame::s_nPlayerModelIndex = nModelID;
	m_bReloadModel = true;
#ifdef GTA_LIBERTY
	m_nModelID = (m_pPed->GetModelIndex() == MI_SPECIAL11) ? MI_SPECIAL10 : MI_SPECIAL11;
//#else
	//m_nModelID = MI_SPECIAL01;
#endif
	CStreaming::FlushRequestList();
	CBaseModelInfo* pInfo = CModelInfo::GetModelInfo(ga_netModelList[nModelID].name, &m_nModelID);
#ifdef GTA_LIBERTY
	if (pInfo && m_nModelID && (m_nModelID < MI_SPECIAL01 || m_nModelID > MI_SPECIAL11))
#else
	if (pInfo && m_nModelID)
#endif
	{
		if (m_pPed->GetModelIndex() == m_nModelID)
		{
			m_bReloadModel = false;
		}
		else if (!CStreaming::HasModelLoaded(m_nModelID))
		{
			CStreaming::RequestModel(m_nModelID, STREAMFLAGS_DEPENDENCY);
		}
	}
	else {
		CStreaming::RequestSpecialModel(m_nModelID, ga_netModelList[nModelID].name, STREAMFLAGS_DEPENDENCY);
	}
	m_nLoadedModelID = m_nModelID;
	m_nMPModelID = cMultiGame::s_nPlayerModelIndex;
}

void cLobbyPed::Update() {
	if (!m_pPed) return;

	/*while (CStreaming::RemoveLoadedZoneModel())
		debug("***** while 10: CStreaming::RemoveLoadedZoneModel()\n");
	for (int nID = 0; nID < NUM_DEFAULT_MODELS; nID++) {
		CStreamingInfo* pInfo = &CStreaming::ms_aInfoForModel[nID];
		if (nID != m_nModelID && CStreaming::IsObjectInCdImage(nID) && pInfo->m_loadState == STREAMSTATE_LOADED && (pInfo->m_flags & STREAMFLAGS_40) == 0) {
			CBaseModelInfo* pInfo = CModelInfo::GetModelInfo(nID);
			if (pInfo && pInfo->GetNumRefs() == 0 && pInfo->GetModelType() == MITYPE_PED) {
				CStreaming::RemoveModel(nID);
			}
		}
	}*/
	CStreaming::LoadRequestedModels();
	if (m_nRequestedModelID >= 0) {
		if (m_nLoadedModelID < 0 || CStreaming::HasModelLoaded(m_nLoadedModelID)) {
			RequestModels(m_nRequestedModelID);
			m_nRequestedModelID = -1;
		}
	}
	if (m_bReloadModel && m_pPed->GetModelIndex() != m_nModelID && CStreaming::HasModelLoaded(m_nModelID)) {
		if (m_fRequestTime == 0.0f || (GET_CURRENT_TIME() - m_fRequestTime) > 0.5f) {
			if (m_pPed->m_rwObject)
				m_pPed->DeleteRwObject();
			m_pPed->m_modelIndex = -1;
			m_pPed->SetModelIndex(m_nModelID);
			m_pPed->m_animGroup = ga_netModelList[m_nMPModelID].bHasSexyWalkAnim ? ASSOCGRP_SEXYWOMAN : ASSOCGRP_GANG1;
			m_bReloadModel = false;
		}
	}
	float fRot = m_fRotation + CTimer::GetTimeStepPauseMode() * 0.5f;
	m_fRotation = fRot > 360.0f ? fRot - 360.0f : fRot;
}

void SetLightsForMultiplayerLobbyPed(float x) {
	RwRGBAReal AmbientLightColourForFrame = { x, x, x };
	RwRGBAReal DirectionalLightColourForFrame = { x, x, x };
	// TODO: missing function call UpdateAmbientColour
	if (pAmbient)
		RpLightSetColor(pAmbient, &AmbientLightColourForFrame);
	if (pDirect)
		RpLightSetColor(pDirect, &DirectionalLightColourForFrame);
}

void cLobbyPed::Render() {
	if (!m_pPed || !m_pPed->m_rwObject)
		return;

#ifdef GTA_LIBERTY
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(m_pHighlightTexture));
	CSprite::RenderOneXLUSprite_Rotate_Aspect(SCREEN_SCALE_X(310.0f), SCREEN_SCALE_Y(100.0f), 12.0f, 50.0f, 125.0f, 255, 255, 255, 220, 0, 0, 128);
#endif
	RwCamera* pCam = Scene.camera;
	RwMatrix mat;
	RwV3d axis1 = { 1.0f, 0.0f, 0.0f };
	RwV3d axis2 = { 0.0f, 0.0f, 1.0f };
	RwV3d transl1 = (-m_pPed->GetPosition());
#ifdef GTA_LIBERTY
	RwV3d transl2 = { 0.94f, 3.65f, -1.0f };
#else
	RwV3d transl2 = { 1.2f, 4.65f, 0.0f }; // PSP -1.2f
#endif
	m_pPed->GetMatrix().CopyToRwMatrix(&mat);
	RwMatrixTranslate(&mat, &transl1, rwCOMBINEPOSTCONCAT);
	RwMatrixRotate(&mat, &axis1, 90.0f, rwCOMBINEPOSTCONCAT);
	RwMatrixTranslate(&mat, &transl2, rwCOMBINEPOSTCONCAT);
	RwMatrixRotate(&mat, &axis2, m_fRotation, rwCOMBINEPOSTCONCAT);
	RwMatrixTranslate(&mat, (RwV3d*)&m_pPed->GetPosition(), rwCOMBINEPOSTCONCAT);
	RwFrameTransform(RwCameraGetFrame(pCam), &mat, rwCOMBINEREPLACE);
	RwFrameUpdateObjects(RwCameraGetFrame(pCam));

	RwCameraEndUpdate(pCam); // TODO!!!! tmp hack to fix 1st frame lobbyped at wrong screen pos, todo mean
	RwCameraBeginUpdate(pCam); // apply cam frame transform
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)true); // TODO!!!! tmp hack to test Z, it is turned on by CSprite2d::DrawRect before CPlayerSkin::RenderFrontendSkinEdit lol
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)true); // same shit (disables at CMenuManager::SetFrontEndRenderStates seems missed in psN)

	if (!RpAnimBlendClumpGetAssociation(m_pPed->GetClump(), ANIM_STD_WALK))
	{
		CAnimBlendAssociation* pAnim = TheAnimManager->AddAnimation(m_pPed->GetClump(), m_pPed->m_animGroup, ANIM_STD_WALK);
		pAnim->SetCurrentTime(m_fWalkAnimTime);
		m_pPed->m_pVehicleAnim = pAnim; // TODO: maybe they created another entry
	}
	CTimer::SetTimeStep(CTimer::GetTimeStepPauseMode());
	m_pPed->bOffscreen = false;
	m_pPed->UpdateAnim();
	CTimer::SetTimeStep(0);
	m_pPed->PreRender();
	SetLightsForMultiplayerLobbyPed(1.0);
	m_pPed->Render();
	if (m_pPed->m_pVehicleAnim)
		m_fWalkAnimTime = m_pPed->m_pVehicleAnim->currentTime;
}

void cLobbyPed::Create() {
	if (m_pPed)
		return;

	CDraw::SetFOV(70.0f);
	m_fWalkAnimTime = 0.0f;
	m_pPed = new CPed(PEDTYPE_CIVMALE);
	m_pPed->SetOrientation(0.0f, 0.0f, 0.0f);
	m_pPed->SetPosition(CVector(0.0f, 0.0f, 0.0f));
	m_pPed->SetPosition(TheCamera.GetPosition()); // hotfix, todo mean why no render
	m_nRequestedModelID = 0;
	m_fRequestTime = 0.0f;
	RequestModels(cMultiGame::s_nPlayerModelIndex);

#ifndef GTA_LIBERTY // leeds optimize gang selection
	for (int32 gang = 0; gang < MP_MAX_GANGS; gang++) {
		gMPGangDefsNetModelListIndices[gang] = 0; // fixbug -1?
		for (int32 netModelListIndex = 0; netModelListIndex < MAX_MP_MODELS; netModelListIndex++) {
			if (ga_netModelList[netModelListIndex].gangID == gang) {
				gMPGangDefsNetModelListIndices[gang] = netModelListIndex;
				break;
			}
		}
	}
#endif
}

void cLobbyPed::Destroy() {
	if (!m_pPed)
		return;
	if (m_pPed->m_rwObject)
		m_pPed->DeleteRwObject();
	delete m_pPed;
	m_pPed = nil;
	m_bReloadModel = false;
	CStreaming::FlushRequestList();
}

cLobbyPed::cLobbyPed() {
	m_pPed = nil;
	m_fRotation = 0.0f;
	m_bReloadModel = false;
#ifdef GTA_LIBERTY
	CTxdStore::PushCurrentTxd();
	int slot = CTxdStore::FindTxdSlot("particle");
	CTxdStore::SetCurrentTxd(slot);
	m_pHighlightTexture = RwTextureRead("player_highlight", nil);
	CTxdStore::PopCurrentTxd();
#else
#ifndef FIX_BUGS // !!!hotfix, FE init2 init lobby, lobby init ped, crash here not found particle, probably different init or unasserted bug
	CTxdStore::PushCurrentTxd();
	int slot = CTxdStore::FindTxdSlot("particle");
	CTxdStore::SetCurrentTxd(slot);
	//m_pHighlightTexture = RwTextureRead("player_highlight", nil);
	CTxdStore::PopCurrentTxd();
#endif
#endif
}

cLobbyPed::~cLobbyPed() {
	Destroy();
#ifdef GTA_LIBERTY
	RwTextureDestroy(m_pHighlightTexture);
#endif
	m_pHighlightTexture = nil;
}