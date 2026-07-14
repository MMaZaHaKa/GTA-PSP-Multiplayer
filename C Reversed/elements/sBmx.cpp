/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "multiplayer/MultiGame.h"
#include "multiplayer/elements/sSyncStream.h"
#include "multiplayer/elements/sBmx.h"


#ifndef GTA_LIBERTY
sBmxSync::sBmxSync() : sVehicleSync() {
	DECLARE_SYNC_CONSTRUCT(this);
	m_fPedalAngleL = 0.0f;
	m_fPedalAngleR = 0.0f;
	m_fCrankAngle = 0.0f;
#ifdef MULTIGAME_BMX_IMPROVEMENTS
	m_fBarSteerAngle = 0.0f;
	m_fLeanAngle = 0.0f;
	for (uint32 i = 0; i < ARRAY_SIZE(m_wheelStatus); i++)
		m_wheelStatus[i] = 0;
#endif
}

sBmxSync::sBmxSync(CBmx* pBmx) : sVehicleSync(pBmx) {
	DECLARE_SYNC_CONSTRUCT(this);
	m_fPedalAngleL = pBmx->m_fPedalAngleL;
	m_fPedalAngleR = pBmx->m_fPedalAngleR;
	m_fCrankAngle = pBmx->m_fCrankAngle;

	//for (uint32 i = 0; i < ARRAY_SIZE(pBmx->m_wheelStatus); i++)
	//	m_wheelStatus[i] = pBmx->m_wheelStatus[i];

	for (uint32 i = 0; i < ARRAY_SIZE(pBmx->m_aWheelTimer); i++)
		m_aWheelTimer[i] = pBmx->m_aWheelTimer[i];

	for (uint32 i = 0; i < ARRAY_SIZE(pBmx->m_aWheelState); i++)
		m_aWheelState[i] = pBmx->m_aWheelState[i]; // 4 - 2

	for (uint32 i = 0; i < ARRAY_SIZE(pBmx->m_aWheelRotation); i++)
		m_aWheelRotation[i] = pBmx->m_aWheelRotation[i]; // 4 - 2

	for (uint32 i = 0; i < ARRAY_SIZE(pBmx->m_aWheelPosition); i++)
		m_aWheelPosition[i] = pBmx->m_aWheelPosition[i]; // 4 - 2

	for (uint32 i = 0; i < ARRAY_SIZE(pBmx->m_aWheelSpeed); i++)
		m_aWheelSpeed[i] = pBmx->m_aWheelSpeed[i]; // 4 - 2

	for (uint32 i = 0; i < ARRAY_SIZE(pBmx->m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = pBmx->m_aSuspensionSpringRatio[i];

	for (uint32 i = 0; i < ARRAY_SIZE(pBmx->m_aWheelColPoints); i++)
		m_aWheelColPoints[i] = pBmx->m_aWheelColPoints[i];

	for (uint32 i = 0; i < ARRAY_SIZE(pBmx->m_aWheelSkidmarkType); i++)
		m_aWheelSkidmarkType[i] = pBmx->m_aWheelSkidmarkType[i];

#ifdef FIX_BUGS
	m_nSkidmarkFlags = 0x0;

	for (uint32 i = 0; i < ARRAY_SIZE(CBmx::m_aWheelSkidmarkBloody); i++) {
		if (pBmx->m_aWheelSkidmarkBloody[i])
			m_nSkidmarkFlags |= BIT(i);
		else
			m_nSkidmarkFlags &= ~BIT(i);

		if (pBmx->m_aWheelSkidmarkUnk[i])
			m_nSkidmarkFlags |= BIT(i + 4);
		else
			m_nSkidmarkFlags &= ~BIT(i + 4);
	}
#endif
#ifdef MULTIGAME_BMX_IMPROVEMENTS
	m_fBarSteerAngle = pBmx->GetRideAnimData()->m_fBarSteerAngle;
	m_fLeanAngle = pBmx->GetRideAnimData()->m_fLeanAngle;
	for (uint32 i = 0; i < ARRAY_SIZE(pBmx->m_wheelStatus); i++)
		m_wheelStatus[i] = pBmx->m_wheelStatus[i];
#endif
}

sBmxSync::sBmxSync(const sBmxSync& other) : sVehicleSync(other) {
	DECLARE_SYNC_CONSTRUCT(this);
#ifdef FIX_BUGS // its right, but in orig its init after creation obj
	m_fPedalAngleL = other.m_fPedalAngleL;
	m_fPedalAngleR = other.m_fPedalAngleR;
	m_fCrankAngle = other.m_fCrankAngle;
#endif
#ifdef MULTIGAME_BMX_IMPROVEMENTS
	m_fBarSteerAngle = other.m_fBarSteerAngle;
	m_fLeanAngle = other.m_fLeanAngle;
	for (uint32 i = 0; i < ARRAY_SIZE(m_wheelStatus); i++)
		m_wheelStatus[i] = other.m_wheelStatus[i];
#endif
}

sBmxSync::~sBmxSync() {
	DECLARE_SYNC_DESTRUCT(this);
}

// inlined
// not checks: m_fPedalAngleL, m_fPedalAngleR, m_fCrankAngle
bool sBmxSync::Compare(const sBmxSync& other)
{
	if (!sVehicleSync::Compare(other))
		return false;

#ifdef FIX_BUGS
	if (m_fPedalAngleL != other.m_fPedalAngleL)
		return false;
	if (m_fPedalAngleR != other.m_fPedalAngleR)
		return false;
	if (m_fCrankAngle != other.m_fCrankAngle)
		return false;
#endif

#ifdef MULTIGAME_BMX_IMPROVEMENTS
	if (m_fBarSteerAngle != other.m_fBarSteerAngle)
		return false;
	if (m_fLeanAngle != other.m_fLeanAngle)
		return false;
	if (memcmp(&m_wheelStatus, &other.m_wheelStatus, sizeof(m_wheelStatus)) != 0)
		return false;
#endif

	return true;
}

#if !defined(FINAL) && !defined(MASTER)
void sBmxSync::Dump()
{
	sVehicleSync::Dump();

	printf("=== sBmxSync Dump ===\n");
	printf("Bmx:\n");
	printf("  m_fPedalAngleL:	%.2f\n", m_fPedalAngleL);
	printf("  m_fPedalAngleR:	%.2f\n", m_fPedalAngleR);
	printf("  m_fCrankAngle:	%.2f\n", m_fCrankAngle);
#ifdef MULTIGAME_BMX_IMPROVEMENTS
	printf("  BarSteerAngle:	%.2f\n", m_fBarSteerAngle);
	printf("  LeanAngle:		%.2f\n", m_fLeanAngle);
	printf("  WheelStatus[0]:	%u (0x%02X)\n", m_wheelStatus[0], m_wheelStatus[0]);
	printf("  WheelStatus[1]:	%u (0x%02X)\n", m_wheelStatus[1], m_wheelStatus[1]);
#endif
	printf("================================\n");
}
#endif


cBmxMG::cBmxMG(sElement* elem) : cVehicleMG(elem)
{
	m_vehType = eVehicleType::VEHICLE_TYPE_BMX;
}

cBmxMG::~cBmxMG()
{

}

void cBmxMG::PreRender(void)
{
	cMultiGame& Game = cMultiGame::Instance();
	sBmx* pBmx = GetElement().bmx;

	// no entity prerender like cBikeMG? lights, etc.

	cVehicleMG::PreRender();

	uint16 nTime = pBmx->m_nTime - static_cast<uint16>(Game.m_nLagValue);
	sBmxSync* bmx = pBmx->FindSync(nTime, nil).bmx;

	CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
	CColModel* colModel = mi->GetColModel();

	// Find contact points for wheel processing
	int32 frontLine = bmx->m_aSuspensionSpringRatio[BIKESUSP_F1] < bmx->m_aSuspensionSpringRatio[BIKESUSP_F2] ? BIKESUSP_F1 : BIKESUSP_F2;
	CVector frontContact(0.0f,
		colModel->lines[BIKESUSP_F1].p0.y,
		colModel->lines[BIKESUSP_F1].p0.z - bmx->m_aSuspensionSpringRatio[frontLine] * pBmx->m_aSuspensionSpringLength[BIKESUSP_F1] - 0.5f * mi->m_wheelScale);
	frontContact = Multiply3x3(GetMatrix(), frontContact);

	int32 rearLine = bmx->m_aSuspensionSpringRatio[BIKESUSP_R1] < bmx->m_aSuspensionSpringRatio[BIKESUSP_R2] ? BIKESUSP_R1 : BIKESUSP_R2;
	CVector rearContact(0.0f,
		colModel->lines[BIKESUSP_R1].p0.y,
		colModel->lines[BIKESUSP_R1].p0.z - bmx->m_aSuspensionSpringRatio[rearLine] * pBmx->m_aSuspensionSpringLength[BIKESUSP_R1] - 0.5f * mi->m_wheelScale);
	rearContact = Multiply3x3(GetMatrix(), rearContact);

	// Update wheel positions from suspension
	float frontWheelPos = colModel->lines[frontLine].p0.z;
	if (bmx->m_aSuspensionSpringRatio[frontLine] > 0.0f)
		frontWheelPos -= bmx->m_aSuspensionSpringRatio[frontLine] * pBmx->m_aSuspensionSpringLength[frontLine];
	bmx->m_aWheelPosition[BIKEWHEEL_FRONT] += (frontWheelPos - bmx->m_aWheelPosition[BIKEWHEEL_FRONT]) * 0.75f;

	float rearWheelPos = colModel->lines[rearLine].p0.z;
	if (bmx->m_aSuspensionSpringRatio[rearLine] > 0.0f)
		rearWheelPos -= bmx->m_aSuspensionSpringRatio[rearLine] * pBmx->m_aSuspensionSpringLength[rearLine];
	bmx->m_aWheelPosition[BIKEWHEEL_REAR] += (rearWheelPos - bmx->m_aWheelPosition[BIKEWHEEL_REAR]) * 0.75f;

	CVector frontWheelFwd = Multiply3x3(GetMatrix(), CVector(-Sin(bmx->m_fSteerAngle), Cos(bmx->m_fSteerAngle), 0.0f));
	CVector rearWheelFwd = GetForward();
	if (bmx->m_aWheelTimer[BIKESUSP_F1] > 0.0f || bmx->m_aWheelTimer[BIKESUSP_F2] > 0.0f)
	{
		float springRatio = Min(pBmx->m_aSuspensionSpringRatio[BIKESUSP_F1], pBmx->m_aSuspensionSpringRatio[BIKESUSP_F2]);
		CVector contactPoint(0.0f,
			(colModel->lines[BIKESUSP_F1].p0.y - colModel->lines[BIKESUSP_F2].p0.y) / 2.0f,
			colModel->lines[BIKESUSP_F1].p0.z - pBmx->m_aSuspensionSpringLength[BIKESUSP_F1] * springRatio - 0.5f * mi->m_wheelScale);
		CVector contactSpeed = GetSpeed(contactPoint) * 0.5f;
		// dirty orig hack, this ctx unused in ProcessWheelRotation
		bmx->m_aWheelSpeed[BIKEWHEEL_FRONT] = ((CVehicle*)nil)->CVehicle::ProcessWheelRotation(WHEEL_STATE_NORMAL, frontWheelFwd, contactSpeed, 0.5f * mi->m_wheelScale);
#ifdef FIX_BUGS
		bmx->m_aWheelRotation[BIKEWHEEL_FRONT] += bmx->m_aWheelSpeed[BIKEWHEEL_FRONT];
#else
		bmx->m_aWheelRotation[BIKEWHEEL_FRONT] += (bmx->m_aWheelSpeed[BIKEWHEEL_FRONT] * CTimer::GetTimeStep()); // bug? ProcessWheelRotation already scaled
#endif
	}

	CMatrix mat;
	CVector pos;

	if (pBmx->m_aBmxNodes[BMX_CHAINSET])
	{
		mat.Attach(RwFrameGetMatrix(pBmx->m_aBmxNodes[BMX_CHAINSET]));
		pos = mat.GetPosition();
		mat.SetRotateX(bmx->m_fCrankAngle);
		mat.Translate(pos);
		mat.UpdateRW();
	}

	if (pBmx->m_aBmxNodes[BMX_PEDAL_R])
	{
		mat.Attach(RwFrameGetMatrix(pBmx->m_aBmxNodes[BMX_PEDAL_R]));
		pos = mat.GetPosition();
		mat.SetRotateX(bmx->m_fPedalAngleL); // L!
		mat.Translate(pos);
		mat.UpdateRW();
	}

	if (pBmx->m_aBmxNodes[BMX_PEDAL_L])
	{
		mat.Attach(RwFrameGetMatrix(pBmx->m_aBmxNodes[BMX_PEDAL_L]));
		pos = mat.GetPosition();
		mat.SetRotateX(bmx->m_fPedalAngleR); // R!
		mat.Translate(pos);
		mat.UpdateRW();
	}

#ifdef MULTIGAME_BMX_IMPROVEMENTS
	// Chassis
	if (pBmx->m_aBmxNodes[BIKE_CHASSIS])
	{
		mat.Attach(RwFrameGetMatrix(pBmx->m_aBmxNodes[BIKE_CHASSIS]));
		pos = mat.GetPosition();
		pos.z = (1.0f - Cos(bmx->m_fLeanAngle)) * (0.9f * colModel->boundingBox.min.z);
		mat.SetRotateX(-0.05f * Abs(bmx->m_fLeanAngle));
		mat.RotateY(bmx->m_fLeanAngle);
		mat.Translate(pos);
		mat.UpdateRW();
	}
#endif
}

// useless override
void cBmxMG::Render(void)
{
	cVehicleMG::Render();
}


sBmx::sBmx() : sVehicle()
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, false);
	for (uint32 i = 0; i < ARRAY_SIZE(m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = 0.0f;
#ifdef FIX_BUGS // kek
	for (uint32 i = 0; i < ARRAY_SIZE(m_aBmxNodes); i++)
		m_aBmxNodes[i] = nil;
#endif
	SetPhysical(new cBmxMG(this));
	DECLARE_ELEMENT_CONSTRUCT(this, false, false);
}

sBmx::sBmx(CBmx* pBmx) : sVehicle()
{
	DECLARE_ELEMENT_CONSTRUCT(this, true, true);
	for (uint32 i = 0; i < ARRAY_SIZE(m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = 0.0f;
#ifdef FIX_BUGS // kek
	for (uint32 i = 0; i < ARRAY_SIZE(m_aBmxNodes); i++)
		m_aBmxNodes[i] = nil;
#endif
	cMultiGame::Instance().AttachEntity(this, pBmx);
	SetEntity(pBmx);
	SetPhysical(new cBmxMG(this));
	Initialise();

//#ifdef FIX_BUGS // recheck
//	// sVehicle
//	m_fTraction = pBmx->m_fTraction;
//	for (uint32 i = 0; i < ARRAY_SIZE(pBmx->m_aSuspensionLineLength); i++)
//		m_aSuspensionLineLength[i] = pBmx->m_aSuspensionLineLength[i];
//	for (uint32 i = 0; i < ARRAY_SIZE(pBmx->m_aSuspensionSpringLength); i++)
//		m_aSuspensionSpringLength[i] = pBmx->m_aSuspensionSpringLength[i];
//	// already in Initialise, but still here to copy sVehicle all info
//	m_aColours[VEHICLE_COLOUR_PRIMARY] = pBmx->m_aColours[VEHICLE_COLOUR_PRIMARY];
//	m_aColours[VEHICLE_COLOUR_SECONDARY] = pBmx->m_aColours[VEHICLE_COLOUR_SECONDARY];
//#endif

	AttachSync(m_nTime, new sBmxSync(pBmx));
	TransferZone();

	// CBmx
//#ifdef FIX_BUGS // orig logic still m_aSuspensionSpringRatio zero // recheck
//	sBmxSync* pSync = GetSync().bmx;
//	for (uint32 i = 0; i < ARRAY_SIZE(pBmx->m_aSuspensionSpringRatio); i++)
//		m_aSuspensionSpringRatio[i] = pSync->m_aSuspensionSpringRatio[i];
//#endif
	DECLARE_ELEMENT_CONSTRUCT(this, false, true);
}

ElementCapability sBmx::GetCapability()
{
	return sBmx::Capability();
}

bool sBmx::HasCapability(ElementCapability capability)
{
	if (sBmx::Capability() == capability)
		return true;
	if (sVehicle::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sBmx::~sBmx()
{
	DECLARE_ELEMENT_DESTRUCT(this);
	sElement::PurgeAttached();
	// ~sElement
}

sElementSync* sBmx::CreateSync() {
	return new sBmxSync();
}

void sBmx::DisposeSync(sElementSync* pSync) {
	if (pSync)
		delete ((sBmxSync*)pSync);
}

sElementSync* sBmx::CreateSyncFromOther(sElementSync* pSync) {
	sBmxSync& sync = *(sBmxSync*)pSync;
#ifdef FIX_BUGS
	return new sBmxSync(sync);
#else
	sBmxSync* pNewSync = new sBmxSync(sync);
	pNewSync->m_fPedalAngleL = sync.m_fPedalAngleL;
	pNewSync->m_fPedalAngleR = sync.m_fPedalAngleR;
	pNewSync->m_fCrankAngle = sync.m_fCrankAngle;
	return pNewSync;
#endif
}

bool sBmx::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) {
	sBmxSync& syncA = *(sBmxSync*)pSyncA;
	sBmxSync& syncB = *(sBmxSync*)pSyncB;
	return syncA.Compare(syncB);
}

void sBmx::ApplyClientSync(uint16 nTime) {
	sVehicle::ApplyClientSync(nTime);

#ifdef FIX_BUGS
	sBmxSync* pSync = GetSync().bmx;
	for (uint32 i = 0; i < ARRAY_SIZE(pSync->m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = pSync->m_aSuspensionSpringRatio[i];
#else
	for (uint32 i = 0; i < ARRAY_SIZE(pSync->m_aSuspensionSpringRatio); i++)
		m_aSuspensionSpringRatio[i] = GetSync().bmx->m_aSuspensionSpringRatio[i];
#endif
}

void sBmx::Update(uint16 nTime) {
	if (GetEntity() != nil)
		AttachSync(nTime, new sBmxSync((CBmx*)GetEntity()));
	else
		delete this; // ?
}

bool sBmx::WriteSyncToStream(sWriteSyncStream* pSyncStream, uint16 nSyncWriteTime, uint16 nSyncLastTime) {
	if (static_cast<int16>(nSyncLastTime - m_vSync.front().m_nTime) >= 0) // FindSync? not GetSyncWithTime?
		return WriteSyncDelta(pSyncStream, FindSync(nSyncWriteTime, nil).bmx, GetSyncWithTime(nSyncLastTime).bmx, (nSyncWriteTime - nSyncLastTime));

	tBmxSyncsDeltas bmxDeltaManager{};
	bmxDeltaManager.SetDifference(); // FindSync? not GetSyncWithTime?
	PerformWriteSync(pSyncStream, FindSync(nSyncWriteTime, nil).bmx, &bmxDeltaManager); // max diff
	return true;
}

void sBmx::ReadSyncFromStream(sReadSyncStream* pSyncStream, sElementSync* pOutSync) {
	sBmxSync& sync = *(sBmxSync*)pOutSync;
	uint16 nDiffMask = pSyncStream->ReadU32();

	if (nDiffMask & eBmxSync::MP_PKTD_BMX_VEHICLE)
		sVehicle::ReadSyncFromStreamVehicle(pSyncStream, (sBmxSync*)pOutSync);

	if (nDiffMask & eBmxSync::MP_PKTD_BMX_PEDAL_ANGLE_L)
		sync.m_fPedalAngleL = pSyncStream->ReadFloat();

	if (nDiffMask & eBmxSync::MP_PKTD_BMX_PEDAL_ANGLE_R)
		sync.m_fPedalAngleR = pSyncStream->ReadFloat();

	if (nDiffMask & eBmxSync::MP_PKTD_BMX_CRANK_ANGLE)
		sync.m_fCrankAngle = pSyncStream->ReadFloat();

#ifdef MULTIGAME_BMX_IMPROVEMENTS
	if (bCurrDestPeerVanilaDevice) // useless, but ok
		return;

	if (nDiffMask & eBmxSync::MP_PKTD_BMX_BAR_STEER_ANGLE)
		sync.m_fBarSteerAngle = pSyncStream->ReadFloat();

	if (nDiffMask & eBmxSync::MP_PKTD_BMX_WHEEL_STATUS_0)
		sync.m_wheelStatus[0] = pSyncStream->ReadU8();

	if (nDiffMask & eBmxSync::MP_PKTD_BMX_WHEEL_STATUS_1)
		sync.m_wheelStatus[1] = pSyncStream->ReadU8();

	if (nDiffMask & eBmxSync::MP_PKTD_BMX_LEAN_ANGLE)
		sync.m_fLeanAngle = pSyncStream->ReadFloat();
#endif
}

void sBmx::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {
	cMultiGame& Game = cMultiGame::Instance();
	if (nOwner != Game.LocalPlayerID()) {
		sVehicle::ReceiveEntity(nOwner, nID, nTime);
		return;
	}

	sBmxSync* pSync = GetSync().bmx;
	assert(GetPhysical());
	CBmx* bmx = new CBmx(GetPhysical()->GetModelIndex(), pSync->VehicleCreatedBy);
	Game.AttachEntity(this, bmx);
	SetEntity(bmx);

#ifdef FIX_BUGS
	for (uint32 i = 0; i < ARRAY_SIZE(bmx->m_aWheelTimer); i++)
		bmx->m_aWheelTimer[i] = pSync->m_aWheelTimer[i];

	for (uint32 i = 0; i < ARRAY_SIZE(bmx->m_aWheelState); i++)
		bmx->m_aWheelState[i] = pSync->m_aWheelState[i]; // 2 - 4

	for (uint32 i = 0; i < ARRAY_SIZE(bmx->m_aWheelRotation); i++)
		bmx->m_aWheelRotation[i] = pSync->m_aWheelRotation[i]; // 2 - 4

	for (uint32 i = 0; i < ARRAY_SIZE(bmx->m_aWheelPosition); i++)
		bmx->m_aWheelPosition[i] = pSync->m_aWheelPosition[i]; // 2 - 4

	for (uint32 i = 0; i < ARRAY_SIZE(bmx->m_aWheelSpeed); i++)
		bmx->m_aWheelSpeed[i] = pSync->m_aWheelSpeed[i]; // 2 - 4

	for (uint32 i = 0; i < ARRAY_SIZE(bmx->m_aSuspensionSpringRatio); i++)
		bmx->m_aSuspensionSpringRatio[i] = pSync->m_aSuspensionSpringRatio[i];

	for (uint32 i = 0; i < ARRAY_SIZE(bmx->m_aWheelColPoints); i++)
		bmx->m_aWheelColPoints[i] = pSync->m_aWheelColPoints[i];

	for (uint32 i = 0; i < ARRAY_SIZE(bmx->m_aWheelSkidmarkType); i++)
		bmx->m_aWheelSkidmarkType[i] = pSync->m_aWheelSkidmarkType[i]; // 2 - 4

	// Skidmark flags
	for (uint32 i = 0; i < ARRAY_SIZE(bmx->m_aWheelSkidmarkBloody); i++) {
		bmx->m_aWheelSkidmarkBloody[i] = (pSync->m_nSkidmarkFlags & BIT(i));
		bmx->m_aWheelSkidmarkUnk[i] = (pSync->m_nSkidmarkFlags & BIT(i + 4));
	}
#endif

#ifdef MULTIGAME_BMX_IMPROVEMENTS
	for (uint32 i = 0; i < ARRAY_SIZE(bmx->m_wheelStatus); i++)
		bmx->m_wheelStatus[i] = pSync->m_wheelStatus[i];

	// Ride Anim Data
	bmx->GetRideAnimData()->m_fLeanAngle = pSync->m_fLeanAngle;
	bmx->GetRideAnimData()->m_fBarSteerAngle = pSync->m_fBarSteerAngle;
#endif

	sVehicle::ReceiveEntity(nOwner, nID, nTime);
#ifdef FIX_BUGS
	bmx->ResetSuspension();
#endif
	AttachSync(m_nTime, new sBmxSync(bmx));
}

void sBmx::Initialise() {
	sVehicle::Initialise();
}

void sBmx::SetupModel() {
	assert(GetPhysical());

	// CBmx::SetupModelNodes
	for (uint32 i = 0; i < ARRAY_SIZE(m_aBmxNodes); i++)
		m_aBmxNodes[i] = nil;
	CClumpModelInfo::FillFrameArray(GetPhysical()->GetClump(), m_aBmxNodes);

	// need?
//#ifdef FIX_BUGS
//	CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetPhysical()->GetModelIndex());
//	tHandlingData* pHandling = GET_HANDLING(GetPhysical()->GetModelIndex());
//	CColModel* colModel = mi->GetColModel();
//	if (colModel->lines == nil)
//	{
//		colModel->numLines = 4;
//		colModel->lines = new CColLine[4];
//	}
//
//	//colModel->lines[0].p0.z = FAKESUSPENSION; // BUG? this would make more sense in the if above
//
//	m_fFrontForkSlope = Tan(DEGTORAD(mi->m_bikeSteerAngle));
//
//	// CBike::SetupSuspensionLines
//	CVector posn;
//	float suspOffset = 0.0f;
//	RwFrame* node = nil;
//	RwMatrix* mat = RwMatrixCreate();
//
//	//bool initialized = colModel->lines[0].p0.z != FAKESUSPENSION;
//
//	for (int32 i = 0; i < 4; i++)
//	{
//		//if (initialized)
//		//{
//		//    posn = colModel->lines[i].p0;
//		//    if (i < 2)
//		//        posn.z = m_aWheelBasePosition[0];
//		//    else
//		//        posn.z = m_aWheelBasePosition[1];
//		//}
//		//else
//		{
//			switch (i)
//			{
//			case BIKESUSP_F1:
//				node = m_aBmxNodes[BIKE_WHEEL_FRONT];
//				suspOffset = 0.25f * mi->m_wheelScale;
//				break;
//			case BIKESUSP_F2:
//				node = m_aBmxNodes[BIKE_WHEEL_FRONT];
//				suspOffset = -0.25f * mi->m_wheelScale;
//				break;
//			case BIKESUSP_R1:
//				node = m_aBmxNodes[BIKE_WHEEL_REAR];
//				suspOffset = 0.25f * mi->m_wheelScale;
//				break;
//			case BIKESUSP_R2:
//				node = m_aBmxNodes[BIKE_WHEEL_REAR];
//				suspOffset = -0.25f * mi->m_wheelScale;
//				break;
//			}
//
//			GetRelativeMatrix(mat, node, node);
//			posn = *RwMatrixGetPos(mat);
//			if (i == BIKESUSP_F1)
//				m_aWheelBasePosition[BIKEWHEEL_FRONT] = posn.z;
//			else if (i == BIKESUSP_R1)
//			{
//				m_aWheelBasePosition[BIKEWHEEL_REAR] = posn.z;
//
//				GetRelativeMatrix(mat, m_aBmxNodes[BIKE_FORKS_REAR], m_aBmxNodes[BIKE_FORKS_REAR]);
//				float dz = posn.z - RwMatrixGetPos(mat)->z;
//				float dy = posn.y - RwMatrixGetPos(mat)->y;
//				m_fRearForkLength = Sqrt(SQR(dy) + SQR(dz));
//				assert(m_fRearForkLength != 0.0f); // we want to divide by this
//			}
//			posn.y += suspOffset;
//		}
//
//		// uppermost wheel position
//		posn.z += pHandling->fSuspensionUpperLimit;
//		colModel->lines[i].p0 = posn;
//
//		// lowermost wheel position
//		posn.z += pHandling->fSuspensionLowerLimit - pHandling->fSuspensionUpperLimit;
//		// lowest point on tyre
//		posn.z -= mi->m_wheelScale * 0.5f;
//		colModel->lines[i].p1 = posn;
//
//		// this is length of the spring at rest
//		m_aSuspensionSpringLength[i] = pHandling->fSuspensionUpperLimit - pHandling->fSuspensionLowerLimit;
//		m_aSuspensionLineLength[i] = colModel->lines[i].p0.z - colModel->lines[i].p1.z;
//	}
//
//	//if (!initialized)
//	{
//		GetRelativeMatrix(mat, m_aBmxNodes[BIKE_FORKS_FRONT], m_aBmxNodes[BIKE_FORKS_FRONT]);
//		m_fFrontForkY = RwMatrixGetPos(mat)->y;
//		m_fFrontForkZ = RwMatrixGetPos(mat)->z;
//	}
//
//	//// Compress spring somewhat to get normal height on road
//	//m_fHeightAboveRoad = m_aSuspensionSpringLength[0] * (1.0f - 1.0f / (4.0f * m_pHandling->fSuspensionForceLevel)) - colModel->lines[0].p0.z + mi->m_wheelScale * 0.5f;
//	//for (i = 0; i < 2; i++)
//	//    m_aWheelPosition[i] = mi->m_wheelScale * 0.5f - m_fHeightAboveRoad;
//
//	// adjust col model to include suspension lines
//	if (colModel->boundingBox.min.z > colModel->lines[0].p1.z)
//		colModel->boundingBox.min.z = colModel->lines[0].p1.z;
//	float radius = Max(colModel->boundingBox.min.Magnitude(), colModel->boundingBox.max.Magnitude());
//	if (colModel->boundingSphere.radius < radius)
//		colModel->boundingSphere.radius = radius;
//
//	RwMatrixDestroy(mat);
//#endif
}

void sBmx::CompareSyncState(sBmxSync* pSync, sBmxSync* pLastSync, uint32 nDelta, tBmxSyncsDeltas* pDiff) {
	sVehicle::CompareSyncState(pSync, pLastSync, nDelta, &pDiff->tVehicleDiff);

	if (pDiff->tVehicleDiff.nVehicleDiff != eVehicleSync::MP_PKTD_VEH_EQUAL)
		pDiff->nBmxDiff |= eBmxSync::MP_PKTD_BMX_VEHICLE;

	// Pedal Angle L
	if (FLT_EPS_NOT_EQ(pLastSync->m_fPedalAngleL, pSync->m_fPedalAngleL))
		pDiff->nBmxDiff |= eBmxSync::MP_PKTD_BMX_PEDAL_ANGLE_L;
#ifdef FIX_BUGS
	else // !!---- UPD!!
		pSync->m_fPedalAngleL = pLastSync->m_fPedalAngleL;
#endif

	// Pedal Angle R
	if (FLT_EPS_NOT_EQ(pLastSync->m_fPedalAngleR, pSync->m_fPedalAngleR))
		pDiff->nBmxDiff |= eBmxSync::MP_PKTD_BMX_PEDAL_ANGLE_R;
#ifdef FIX_BUGS
	else // !!---- UPD!!
		pSync->m_fPedalAngleR = pLastSync->m_fPedalAngleR;
#endif

	// Crank Angle
	if (FLT_EPS_NOT_EQ(pLastSync->m_fCrankAngle, pSync->m_fCrankAngle))
		pDiff->nBmxDiff |= eBmxSync::MP_PKTD_BMX_CRANK_ANGLE;
#ifdef FIX_BUGS
	else // !!---- UPD!!
		pSync->m_fCrankAngle = pLastSync->m_fCrankAngle;
#endif

#ifdef MULTIGAME_BMX_IMPROVEMENTS
	if (bCurrDestPeerVanilaDevice)
		return;

	// Bar steer angle
	if (FLT_EPS_NOT_EQ(pLastSync->m_fBarSteerAngle, pSync->m_fBarSteerAngle))
		pDiff->nBmxDiff |= eBmxSync::MP_PKTD_BMX_BAR_STEER_ANGLE;
	else // !!---- UPD!!
		pSync->m_fBarSteerAngle = pLastSync->m_fBarSteerAngle;

	// Wheel status 0
	if (pLastSync->m_wheelStatus[0] != pSync->m_wheelStatus[0])
		pDiff->nBmxDiff |= eBmxSync::MP_PKTD_BMX_WHEEL_STATUS_0;

	// Wheel status 1
	if (pLastSync->m_wheelStatus[1] != pSync->m_wheelStatus[1])
		pDiff->nBmxDiff |= eBmxSync::MP_PKTD_BMX_WHEEL_STATUS_1;

	// Lean angle
	if (pLastSync->m_fLeanAngle != pSync->m_fLeanAngle)
		pDiff->nBmxDiff |= eBmxSync::MP_PKTD_BMX_LEAN_ANGLE;
#endif
}

void sBmx::PerformWriteSync(sWriteSyncStream* pSyncStream, sBmxSync* pSync, tBmxSyncsDeltas* pDiff) {
	pSyncStream->WriteU32(pDiff->nBmxDiff);

	if (pDiff->nBmxDiff & eBmxSync::MP_PKTD_BMX_VEHICLE)
		sVehicle::PerformWriteSync(pSyncStream, pSync, &pDiff->tVehicleDiff);

	if (pDiff->nBmxDiff & eBmxSync::MP_PKTD_BMX_PEDAL_ANGLE_L)
		pSyncStream->WriteFloat(pSync->m_fPedalAngleL);

	if (pDiff->nBmxDiff & eBmxSync::MP_PKTD_BMX_PEDAL_ANGLE_R)
		pSyncStream->WriteFloat(pSync->m_fPedalAngleR);

	if (pDiff->nBmxDiff & eBmxSync::MP_PKTD_BMX_CRANK_ANGLE)
		pSyncStream->WriteFloat(pSync->m_fCrankAngle);

#ifdef MULTIGAME_BMX_IMPROVEMENTS
	if (bCurrDestPeerVanilaDevice)
		return;

	if (pDiff->nBmxDiff & eBmxSync::MP_PKTD_BMX_BAR_STEER_ANGLE)
		pSyncStream->WriteFloat(pSync->m_fBarSteerAngle);

	if (pDiff->nBmxDiff & eBmxSync::MP_PKTD_BMX_WHEEL_STATUS_0)
		pSyncStream->WriteU8(pSync->m_wheelStatus[0]);

	if (pDiff->nBmxDiff & eBmxSync::MP_PKTD_BMX_WHEEL_STATUS_1)
		pSyncStream->WriteU8(pSync->m_wheelStatus[1]);

	if (pDiff->nBmxDiff & eBmxSync::MP_PKTD_BMX_LEAN_ANGLE)
		pSyncStream->WriteFloat(pSync->m_fLeanAngle);
#endif
}

bool sBmx::WriteSyncDelta(sWriteSyncStream* pSyncStream, sBmxSync* pSync, sBmxSync* pLastSync, uint32 nDelta) {
	tBmxSyncsDeltas bmxDeltaManager{};
	bmxDeltaManager.SetEqual();
	CompareSyncState(pSync, pLastSync, nDelta, &bmxDeltaManager);

#ifdef FIX_BUGS
	if (bmxDeltaManager.nBmxDiff == eBmxSync::MP_PKTD_BMX_EQUAL) // main delta
		return false;
#endif

	PerformWriteSync(pSyncStream, pSync, &bmxDeltaManager);
	return true;
}

#ifdef FIX_BUGS
void sBmx::Fix() {
	;
}
#endif

#endif