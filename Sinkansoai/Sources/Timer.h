#pragma once
#include <chrono>
#include "Module.h"

class MTimer : public MModuleBase
{
	MODULE_CLASS_DECORATOR(MTimer)

private:
	// Refactor as platform specific timer
	std::chrono::time_point<std::chrono::high_resolution_clock> CurrentTime{};
	std::chrono::time_point<std::chrono::high_resolution_clock> PreviousTime{};
	std::chrono::duration<float> DeltaTime{};

public:

	virtual void Init() override;

	virtual void Teardown() override;

	virtual void Update() override;

	float GetDelta() const { return DeltaTime.count(); }
};

