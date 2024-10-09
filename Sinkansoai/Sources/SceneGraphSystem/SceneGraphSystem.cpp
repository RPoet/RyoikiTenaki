#include "SceneGraphSystem.h"

void MSceneGraphSystem::RegisterEntity(MGraphEntity* Entity)
{
	EntitiesToRegister.emplace(Entity);
}

void MSceneGraphSystem::DestoryEntity(MGraphEntity* Entity)
{
	EntitiesToDestroy.emplace(Entity);
}

void MSceneGraphSystem::FlushEntityContext()
{
	for (auto& Entity : EntitiesToDestroy)
	{
		if (Entity && Entity->IsRegistered())
		{
			Entity->Destroy();
			// TO DO : Improve delete algorithm.
			GraphEntities.erase(std::remove(GraphEntities.begin(), GraphEntities.end(), Entity), GraphEntities.end());
		}
	}

	for (auto& Entity : EntitiesToRegister)
	{
		if (Entity && !Entity->IsRegistered())
		{
			Entity->Register();
			GraphEntities.push_back(Entity);
		}
	}

	EntitiesToDestroy.clear();
	EntitiesToRegister.clear();
}

void MSceneGraphSystem::Tick(float DeltaTime)
{
	FlushEntityContext();

	for (auto& Entity : GraphEntities)
	{
		Entity->Tick(DeltaTime);
	}
}


void MSceneGraphSystem::Serialize()
{
	for (auto& Entity : EntitiesToRegister)
	{
		Entity->Serialize();
	}
}

void MSceneGraphSystem::Teardown()
{
	cout << "SceneGraphSystem Teardown" << endl;

	cout << "Before GraphEntities : "<< GraphEntities.size() << endl;

	for (auto& Entity : GraphEntities)
	{
		if (Entity && (!Entity->IsExternalObject()))
		{
			delete Entity;
		}
	}

	cout << "After GraphEntities : "<< GraphEntities.size() << endl;
}
