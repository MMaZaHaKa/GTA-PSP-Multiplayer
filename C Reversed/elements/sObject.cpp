/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"

#include "multiplayer/elements/sObject.h"
#include "multiplayer/MultiGame.h"

#ifdef MULTIGAME_IMPROVEMENTS

sObjectSync::sObjectSync() : sElementPhysicalSync()
{

}

sObjectSync::sObjectSync(CObject* pObject) : sElementPhysicalSync(pObject)
{

}

sObjectSync::sObjectSync(const sObjectSync& other) : sElementPhysicalSync(other)
{

}

sObjectSync::~sObjectSync()
{

}

bool sObjectSync::Compare(const sObjectSync& other)
{
	TODO();
	TODO();
	TODO();
	TODO();
	return false;
}


sObject::sObject() {

}

sObject::sObject(CObject* object) {

}

ElementCapability sObject::GetCapability()
{
	return sObject::Capability();
}

bool sObject::HasCapability(ElementCapability capability)
{
	if (sObject::Capability() == capability)
		return true;
	if (sElementPhysical::Capability() == capability)
		return true;
	if (sElement::Capability() == capability)
		return true;
	return false;
}

sObject::~sObject() {
	
}

sElementSync* sObject::CreateSync() {
	return new sObjectSync();
}

void sObject::DisposeSync(sElementSync* pSync) {
	delete (sObjectSync*)pSync;
}

sElementSync* sObject::CreateSyncFromOther(sElementSync* pSync)
{
	sObjectSync& sync = *(sObjectSync*)pSync;
	return new sObjectSync(sync);
}

bool sObject::HasSyncChanged(sElementSync* pSyncA, sElementSync* pSyncB)
{
	sObjectSync& syncA = *(sObjectSync*)pSyncA;
	sObjectSync& syncB = *(sObjectSync*)pSyncB;
	return syncA.Compare(syncB);
}
#endif