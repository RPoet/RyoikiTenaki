#pragma once
#include <chrono>
#include "Definitions.h"

class MTimer
{
private:
	// Refactor as platform specific timer
	std::chrono::time_point<std::chrono::high_resolution_clock> CurrentTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> PreviousTime;
	std::chrono::duration<float> DeltaTime;

public:
	void Init();
	void Exit();
	void Tick();

	float GetDelta() const { return DeltaTime.count(); }
};