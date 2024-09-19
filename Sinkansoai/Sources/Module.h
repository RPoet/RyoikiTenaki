#pragma once
#include "Definitions.h"
#include "ObjectBase.h"

// To register module into engine.
#include "Engine.h"

class MModuleBase : public MObjectBase
{
protected:
	CLASS_DECORATOR(MModuleBase)

protected:

	int32 bIsInitialized : 1;

public:

	MModuleBase();

	virtual void Init()
	{
		bIsInitialized = true;
	};

	virtual void Teardown() 
	{
		bIsInitialized = false;
	};

	virtual void Update() {};

	void Check()
	{
		assert(bIsInitialized);
	}
};
