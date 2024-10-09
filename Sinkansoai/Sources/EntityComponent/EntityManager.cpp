#include <windows.h>
#include "EntityManager.h"

MEntityManager::MEntityManager()
{
	Entities.resize(ECF::NumMaxEntity);

	uint32 LastAllocatedId = 1;
	for (auto& Entity : Entities)
	{
		Entity.Id = LastAllocatedId;

		++LastAllocatedId;
	}
}

MEntityData MEntityManager::AllocateEntity()
{
	const int32 LastIndex = Entities.size() - 1;
	assert(LastIndex >= 0 && "  Used entity is greater than limited  ");

	++NumEntities;
	return Entities[LastIndex];
}

void MEntityManager::DeallocateEntity(MEntityData& Entity)
{
	Entities.push_back(Entity);	
	--NumEntities;
}
