#pragma once
#include "Module.h"

class MInput: public MModuleBase
{
	MODULE_CLASS_DECORATOR(MInput)

private:
	enum 
	{ 
		KEY_UP,
		KEY_UP_CONT,
		KEY_DOWN,
		KEY_DOWN_CONT
	};

	int32 Key[256]{};
public:

	virtual void Init() override;
	virtual void Teardown() override;
	virtual void Update() override;

	inline bool IsPressed(int InKey)
	{
		return Key[InKey] >= KEY_DOWN;
	}
};
