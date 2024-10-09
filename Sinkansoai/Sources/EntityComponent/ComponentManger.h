#pragma once
#include "ECFCommon.h"
#include "../Singleton.h"


class IComponent
{
public:
	virtual ~IComponent() = default;
	virtual void Destroy(const MEntityData& entity) = 0;
	virtual void Tick(float DeltaTime) {};
};

template< class T >
class MComponent : public IComponent
{
private:
	unordered_map<uint32, uint32> EntityIdToComponentIndex;
	unordered_map<uint32, uint32> ComponentIndextoEntityId;

	vector<T> Datum;

public:
	MComponent() = default;
	~MComponent() = default;



	void Destroy(const MEntityData& entity) override
	{

	}
};
