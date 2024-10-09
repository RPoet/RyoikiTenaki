#pragma once
#include "ECFCommon.h"
#include "../ObjectBase.h"

class MEntity : public MObjectBase
{
protected:

	CLASS_DECORATOR(MEntity)

private:	
	MEntityData Data;
	int32 bTickable : 1;

public:
	MEntity();

	bool IsTickable() const { return bTickable; }
	             
	void SetTickable(bool bTickable)
	{
		this->bTickable = bTickable;
	}

	virtual void Register();

	virtual void Destroy();

	virtual void Tick(float DeltaTime) {};
};

