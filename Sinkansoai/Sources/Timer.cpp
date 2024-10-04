#include "Timer.h"
#include "Engine.h"

IMPLEMENT_MODULE(MTimer)

void MTimer::Init()
{
	Super::Init();
	
	cout << "Timer Init" << endl;

	// Timer always updated first.
	Priority = EModulePriority::EHigh;

	CurrentTime = std::chrono::high_resolution_clock::now();
	PreviousTime = std::chrono::high_resolution_clock::now();
}


void MTimer::Teardown()
{
	Super::Teardown();
}

void MTimer::Update()
{
	CurrentTime = std::chrono::high_resolution_clock::now();
	DeltaTime = (CurrentTime - PreviousTime);
	PreviousTime = CurrentTime;
}

