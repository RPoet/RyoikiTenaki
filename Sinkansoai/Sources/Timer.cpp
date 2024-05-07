#include "Timer.h"

void MTimer::Init()
{
	CurrentTime = std::chrono::high_resolution_clock::now();
	PreviousTime = std::chrono::high_resolution_clock::now();
}


void MTimer::Exit()
{
}

void MTimer::Tick()
{
	CurrentTime = std::chrono::high_resolution_clock::now();
	DeltaTime = (CurrentTime - PreviousTime);
	PreviousTime = CurrentTime;
}

