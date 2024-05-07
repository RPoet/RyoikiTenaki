#pragma once
#include "Definitions.h"
#include "Timer.h"
#include "Input.h"

class MEngine
{
private:
	MTimer Timer;
	MInput Input;


	float DeltaTime;
public:

	void Init();
	void Exit();

	void Loop();
};