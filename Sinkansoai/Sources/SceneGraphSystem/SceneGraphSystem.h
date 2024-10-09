#pragma once
#include "../Definitions.h"
#include "GraphEntity.h"


// Make this abstart to use this system as polymorphic system
// And later both ECS and Scene graph would be compared in performance wise
class MSceneGraphSystem
{
protected:

	// Polymorphism entities.
	// TO DO make sure working with pool system.
	vector< MGraphEntity* > GraphEntities;	
	
	unordered_set< MGraphEntity* > EntitiesToRegister;
	unordered_set< MGraphEntity* > EntitiesToDestroy;

public:
	MSceneGraphSystem() = default;
	//template<typename T, typename... Ts>
	//void CreateEntity(Ts&&... Args)
	//{
	//}

	void RegisterEntity(MGraphEntity*);
	void DestoryEntity(MGraphEntity*);

	void FlushEntityContext();
	void Tick(float DeltaTime);

	void Serialize();
	void Teardown();
};
