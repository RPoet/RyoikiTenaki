#pragma once
#include "Definitions.h"
#include "ObjectBase.h"

class MComponentBase : public MObjectBase
{
protected:

	CLASS_DECORATOR(MComponentBase)

private:
	int32 bTickable : 1;

public:
	MComponentBase();

	bool IsTickable() const { return bTickable; }

	void SetTickable(bool bTickable)
	{
		this->bTickable = bTickable;
	}

	virtual void Register() {};

	virtual void Destroy() {};

	virtual void Tick() {};
};

