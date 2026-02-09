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

	int32 MouseX = 0;
	int32 MouseY = 0;
	int32 MouseDeltaX = 0;
	int32 MouseDeltaY = 0;
	int32 PrevMouseX = 0;
	int32 PrevMouseY = 0;

	bool bLeftButtonDown = false;
	bool bRightButtonDown = false;
	bool bMiddleButtonDown = false;

public:

	virtual void Init() override;
	virtual void Teardown() override;
	virtual void Update() override;

	inline bool IsPressed(int InKey)
	{
		return Key[InKey] >= KEY_DOWN;
	}

	inline bool IsLeftMouseDown() const { return bLeftButtonDown; }
	inline bool IsRightMouseDown() const { return bRightButtonDown; }
	inline bool IsMiddleMouseDown() const { return bMiddleButtonDown; }

	inline int32 GetMouseX() const { return MouseX; }
	inline int32 GetMouseY() const { return MouseY; }
	inline int32 GetMouseDeltaX() const { return MouseDeltaX; }
	inline int32 GetMouseDeltaY() const { return MouseDeltaY; }
};
