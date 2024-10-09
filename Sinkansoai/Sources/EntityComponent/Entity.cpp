#include <windows.h>
#include "Entity.h"
#include "EntityManager.h"

MEntity::MEntity()
	: Super()
	, Data()
	, bTickable(true)
{
}

void MEntity::Register()
{
	Data = MEntityManager::Get().AllocateEntity();
}

void MEntity::Destroy()
{
	MEntityManager::Get().DeallocateEntity(Data);
}
