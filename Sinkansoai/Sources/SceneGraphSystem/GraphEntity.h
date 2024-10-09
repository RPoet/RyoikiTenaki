#pragma once
#include "../Definitions.h"
#include "../ObjectBase.h"
#include "Component.h"

class MGraphEntity : public MObjectBase
{
	CLASS_DECORATOR(MGraphEntity)

protected:
	int32 bTickable : 1;
	int32 bExternalObject : 1;

	vector<IComponent*> Components;
	//vector<MGraphEntity*> Children;

public:
	MGraphEntity();
	virtual ~MGraphEntity() = default;

	bool IsTickable() const
	{
		return bTickable;
	}

	void SetTickable(bool bTickable)
	{
		this->bTickable = bTickable;
	}

	bool IsExternalObject() const
	{
		return bExternalObject;
	}

	void SetExternalObject(bool bExternalObject)
	{
		this->bExternalObject = bExternalObject;
	}

	virtual void Register() 
	{
		SetRegistered(true);
	};

	virtual void Destroy()
	{
		SetRegistered(false);
	};

	virtual void Tick(float DeltaTime) {};
};

