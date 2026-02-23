/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "common.h"
#include "Object.h"
#include "multiplayer/elements/sElementPhysical.h"

#ifdef MULTIGAME_IMPROVEMENTS
struct sObjectSync : sElementPhysicalSync {
public:

	sObjectSync();
	sObjectSync(CObject* pObject);
	sObjectSync(const sObjectSync& other);
	~sObjectSync() override;

	bool Compare(const sObjectSync& other);
};

class cObjectMG : public cPhysicalMG {
private:

};

struct sObject : sElementPhysical {
public:

	sObject();
	sObject(CObject* object);

	static ElementCapability Capability() { return reinterpret_cast<ElementCapability>(sObject::Capability); }

	ElementCapability GetCapability() override;
	bool HasCapability(ElementCapability capability) override;
	~sObject() override;
	sElementSync* CreateSync() override;
	void DisposeSync(sElementSync* pSync) override;
	sElementSync* CreateSyncFromOther(sElementSync* pSync) override;
	bool HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB) override;
	eElementType GetType() override { return eElementType::ELEMENT_TYPE_OBJECT; }

};
#endif