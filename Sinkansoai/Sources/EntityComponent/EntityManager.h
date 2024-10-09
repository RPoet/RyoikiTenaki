#pragma once
#include "ECFCommon.h"
#include "../Singleton.h"

class MEntityManager : public Singleton<MEntityManager>
{
private:

	vector<MEntityData> Entities;

	int32 NumEntities = 0;
public:

	MEntityManager();
	
	MEntityData AllocateEntity();

	void DeallocateEntity(MEntityData& Entity);
};
