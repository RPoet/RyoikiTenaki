#pragma once
#include "Definitions.h"

class MInput
{
private:
	enum 
	{ 
		KEY_UP,
		KEY_UP_CONT,
		KEY_DOWN,
		KEY_DOWN_CONT
	};

	int Key[256];
public:

	void Init();
	void Exit();
	void Tick();

	inline bool IsPressed(int InKey)
	{
		return Key[InKey] >= KEY_DOWN;
	}
};
