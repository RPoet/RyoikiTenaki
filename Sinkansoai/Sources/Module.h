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

	enum class EModulePriority : int8
	{
		EHigh = -1,
		EMid = 0,
		ELow = 1
	};

	EModulePriority Priority{ EModulePriority::ELow };

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

	EModulePriority GetPriority() const { return Priority; }

	virtual void PrintName() { cout << "DefaultModule" << endl; };
	void Check()
	{
		assert(bIsInitialized);
	}

};
