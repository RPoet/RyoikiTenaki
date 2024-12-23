#pragma once
#include "ECFCommon.h"
#include "../Singleton.h"


class IComponentECS
{
public:
	virtual ~IComponentECS() = default;
	virtual void Destroy(const MEntityData& entity) = 0;
	virtual void Tick(float DeltaTime) {};
};

template< class T >
class MComponentECS : public IComponentECS
{
private:
	unordered_map<uint32, uint32> EntityIdToComponentIndex;
	unordered_map<uint32, uint32> ComponentIndextoEntityId;

	vector<T> Datum;

public:
	MComponentECS() = default;
	~MComponentECS() = default;

	void Destroy(const MEntityData& entity) override
	{

	}
};
