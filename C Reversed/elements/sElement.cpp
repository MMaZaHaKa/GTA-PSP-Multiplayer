/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "World.h"

#include "multiplayer/elements/sElement.h"
#include "multiplayer/MultiGame.h"

sElement::sElement() {
	cMultiGame& Game = cMultiGame::Instance();
	m_bWasTransfered = false;
	m_nPrevOwnerID = -1;
	m_bIsNewSync = false;
	m_nOwnerID = Game.LocalPlayerID();
	m_nPrevID = -1;
	m_nID = 0;
	m_nTime = Game.m_pNetSession->m_nCurTime;
	m_nLastSentFrame = 0;
	m_nDeltaTime = Game.m_pNetSession->m_nCurTime;
	m_vSync = std::deque<tSyncEntry>();
	m_vSyncB = std::deque<tSyncEntry>();
	m_pSync = { nil };
	m_pZone = nil;
	SetEntity(nil);
	if (Game.IsOpen())
		m_nLastSentFrame = Game.m_pNetSession->m_nCurTime;
}

ElementCapability sElement::GetCapability()
{
	return sElement::Capability();
}

bool sElement::HasCapability(ElementCapability capability)
{
	return sElement::Capability() == capability;
}

// custom
void sElement::PurgeAttached() {
	for (auto& e : m_vSync)
		DisposeSync(e.m_pAttachedElement.element);
	for (auto& e : m_vSyncB)
		DisposeSync(e.m_pAttachedElement.element);

	m_vSync.clear();
	m_vSyncB.clear();

	m_pSync = { nil };
	m_bIsNewSync = false;
}

sElement::~sElement()
{
	cMultiGame& Game = cMultiGame::Instance();
	Game.RemoveElement(this);
	if (m_pZone && m_nOwnerID == Game.LocalPlayerID())
		m_pZone->RemoveElement(this);
	if (GetEntity() != nil) {
		CWorld::Remove(GetEntity());
		delete m_pEntity; // warn! vanila delete CWorld::Players[index].m_pPed
#ifdef FIX_BUGS
		SetEntity(nil);
#endif
	}
}

void sElement::ApplyClientSync(uint16 time) {
	m_nTime = time;

	uint16 lagAdjustedTime = m_nTime - static_cast<uint16>(TheMPGame.m_nLagValue);

	while (!m_vSyncB.empty()) {
		tSyncEntry& frontEntry = m_vSyncB.front();
		if (frontEntry.m_nTime >= lagAdjustedTime) {
			break;
		}
		DisposeSync(frontEntry.m_pAttachedElement.element);
		m_vSyncB.pop_front();
	}

	bool isNewSync = false;
	m_pSync = FindSync(time, &isNewSync);
	m_bIsNewSync = isNewSync;
}

void sElement::Update(uint16 time) {
	AttachSync(time, CreateSyncFromOther(FindSync(time, nil).element));
}

void sElement::ReceiveEntity(uint8 nOwner, uint16 nID, uint16 nTime) {
	m_pZone = nil;
	m_nPrevOwnerID = m_nOwnerID;
	m_nPrevID = m_nID;
	m_nOwnerID = nOwner;
	m_nID = nID;
	m_nTime = nTime;
	m_nDeltaTime = nTime;

	PurgeAttached();
}

void sElement::UpdateDelta(sElementSync* pSync, uint16 nTimeDelta) {
	;
}

void sElement::ApplyDeltaState(sElementSync* pSync, uint16 nTimeDelta) {
	UpdateDelta(pSync, nTimeDelta);
}

void sElement::InterpolateDeltaState(sElementSync* pSyncA, sElementSync* pSyncB, float fDelta) {
	;
}

void sElement::TransferEntity(int16 nDestPlayer) {
	m_bWasTransfered = true;
	debug("TransferEntity %i, %i to %i\n", GetOwner(), GetID(), nDestPlayer);
	cMultiGame::Instance().SendTransferPacket(this, nDestPlayer);
}

void sElement::RegisterSelf() {
	cMultiGame::Instance().RegisterEntity(this);
}

void sElement::RegisterSelfWithOwner(uint8 nOwner, uint16 nID) {
	m_nOwnerID = nOwner;
	m_nID = nID;
	cMultiGame::Instance().RegisterEntity(this);
}

void sElement::AttachSync(uint16 time, sElementSync* pSync) {
	if (!m_vSync.empty() && static_cast<int16>(m_nDeltaTime - time) >= 0) {
		DisposeSync(pSync);
	}
	else {
		DisposeAttachedDelta(time, time + 1);
		m_vSync.push_back({ time, { pSync } });
	}
}

uElementSync sElement::FindSync(uint16 time, bool* bIsNewSync) {
	auto lb_it = m_vSyncB.begin();
	for (; lb_it != m_vSyncB.end(); ++lb_it) {
		if (static_cast<int16>(lb_it->m_nTime - time) >= 0)
			break;
	}
	if (lb_it != m_vSyncB.end() && lb_it->m_nTime == time) {
		if (bIsNewSync) *bIsNewSync = true;
		return lb_it->m_pAttachedElement;
	}

	auto upper_it = m_vSync.begin();
	for (; upper_it != m_vSync.end(); ++upper_it) {
		if (static_cast<int16>(upper_it->m_nTime - time) < 0)
			break;
	}

	if (upper_it == m_vSync.begin()) {
		if (m_vSync.empty()) {
			if (bIsNewSync) *bIsNewSync = true;
			return { nil };
		}
		if (bIsNewSync) *bIsNewSync = false;
		return m_vSync.front().m_pAttachedElement;
	}

	auto prev_it = upper_it - 1;
	uint16 prev_time = prev_it->m_nTime;
	uElementSync prev_sync = prev_it->m_pAttachedElement;

	if (prev_time == time) {
		if (bIsNewSync) *bIsNewSync = false;
		return prev_sync;
	}

	uElementSync new_sync;
	new_sync.element = CreateSyncFromOther(prev_sync.element);

	m_vSyncB.insert(lb_it, { time, new_sync });

	int16 delta = static_cast<int16>(time - prev_time);
	if (delta > 0)
		UpdateDelta(new_sync.element, delta);

	if (upper_it != m_vSync.end()) {
		uint16 next_time = upper_it->m_nTime;
		uElementSync next_sync = upper_it->m_pAttachedElement;
		float factor = static_cast<float>(delta) / static_cast<float>(next_time - prev_time);
		InterpolateDeltaState(new_sync.element, next_sync.element, factor);
	}
	else if (delta > 0)
		ApplyDeltaState(new_sync.element, delta);

	if (bIsNewSync) *bIsNewSync = true;
	return new_sync;
}

uElementSync sElement::GetSyncWithTime(uint16 time) {
	for (const tSyncEntry& entry : m_vSync) {
		if (entry.m_nTime == time)
			return entry.m_pAttachedElement;
	}
	assert(false && "syn not found");
	return { nil };
}

void sElement::RegisterZone(cInterestZone* zone) {
	cInterestZone* pPrevZone = m_pZone;
	if (zone == pPrevZone) return;
	bool same_peers = false;
	if (pPrevZone) {
		m_pZone = nil;
		pPrevZone->RemoveElement(this);
		uint32 prev_count = pPrevZone->m_vPeers.size();
		if (zone && prev_count == zone->m_vPeers.size()) {
			same_peers = true;
			for (uint32 i = 0; i < prev_count; ++i) {
				uint16 peer_id = pPrevZone->m_vPeers[i].nPeerID;
				if (!zone || !zone->HasPeer(peer_id)) {
					same_peers = false;
				}
			}
		}
	}
	if (!same_peers)
		ClearSyncs();
	m_pZone = zone;
}

sPlayer* sElement::GetOwnerPlayer() {
	return cMultiGame::Instance().GetPlayer(GetOwner());
}

void sElement::ClearSyncs() {
	if (m_vSync.empty() || m_vSync.size() == 1)
		return;
	m_vSync.erase(m_vSync.begin(), m_vSync.end() - 1);
}

void sElement::DisposeAttachedDelta(uint16 nNetTime, uint16 nZoneTime) {
	if (static_cast<int16>(nNetTime - nZoneTime) >= 0)
		DisposeAttached(nZoneTime);

	if (!cMultiGame::Instance().IsPlayerConnected(GetOwner()))
		return;

	if (m_vSync.empty() || static_cast<int16>(nNetTime - m_nDeltaTime) >= 0) {
		m_nDeltaTime = nNetTime;
		while (!m_vSyncB.empty() && static_cast<int16>(m_nDeltaTime - m_vSyncB.front().m_nTime) >= 0) {
			if (m_pSync.element == m_vSyncB.front().m_pAttachedElement.element)
				m_pSync.element = nil;
			DisposeSync(m_vSyncB.front().m_pAttachedElement.element);
			m_vSyncB.pop_front();
		}
	}
}

void sElement::DisposeAttached(uint16 nTime) {
	uint16 adjustedTime = nTime;
	if (static_cast<int16>(m_nTime - nTime) < 0)
		adjustedTime = m_nTime;

	while (m_vSync.size() >= 2 && static_cast<int16>(m_vSync.front().m_nTime - adjustedTime) < 0) {
		if (m_pSync.element == m_vSync.front().m_pAttachedElement.element)
			m_pSync.element = nil;
		DisposeSync(m_vSync.front().m_pAttachedElement.element);
		m_vSync.pop_front();
	}
}

uElementSync sElement::GetSyncWithTime2(uint16 nState, uint16 nBasis) {
	DisposeAttachedDelta(nState, nBasis);

	uElementSync new_sync;
	uint16 insert_time;
	if (nBasis >= nState) {
		new_sync.element = CreateSync();
		insert_time = nState;
	}
	else // nState > nBasis
	{
		auto it = m_vSync.begin();
		assert(it != m_vSync.end()); // crash ++next_it; // cant apply delta for this state
		while (true) {
			auto next_it = it;
			++next_it;

			if (next_it == m_vSync.end())
				break;

			if (nBasis < next_it->m_nTime)
				break;

			it = next_it;
		}

		new_sync.element = CreateSyncFromOther(it->m_pAttachedElement.element);
		uint16 found_time = it->m_nTime;
		UpdateDelta(new_sync.element, nState - found_time);
		insert_time = nState;
	}

	tSyncEntry new_entry{ insert_time, new_sync };
	auto insert_pos = std::lower_bound(m_vSync.begin(), m_vSync.end(), new_entry,
		[](const tSyncEntry& a, const tSyncEntry& b) {
			return a.m_nTime < b.m_nTime;
		});
	m_vSync.insert(insert_pos, new_entry);
	bPhyUnk_1 = false;
	return new_sync;
}

void sElement::DisposeFrame(uint16 nTime) {
	if (m_vSync.empty()) {
		return;
	}
	auto it = m_vSync.end();
	do {
		--it;
		if (it->m_nTime == nTime) {
			break;
		}
	} while (it != m_vSync.begin());

	if (it != m_vSync.begin() && it->m_nTime == nTime) {
		auto prev_it = it - 1;
		prev_it->m_pAttachedElement.element->m_bUnk = true;
	}
}

sPeerState* sElement::GetPeer() {
#ifdef GTA_LIBERTY
	TODO(); // from m_vPlayers multigame
#else
	return PeerManager.GetPeerById(GetOwner()); 
#endif
}

uint32 sElement::GetSyncCount(bool owned) {
	uint32 totalSyncs = 0;
	for (auto& elemPair : GetPeer()->m_vElements) {
		sElement* pElement = elemPair.second;
		if (pElement && (!owned || pElement == this))
			totalSyncs += pElement->m_vSync.size();
	}
	return totalSyncs;
}

bool gb_mp_will_destroy_elem = false;