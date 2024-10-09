#include "Launcher.h"
#include "Engine.h"
#include "Timer.h"
#include "World.h"


IMPLEMENT_MODULE(MLauncher)

void MLauncher::Init()
{
	Super::Init();
	cout << "Launcher Init " << endl;
	GenerateWorld();
}


void MLauncher::Teardown()
{
	Super::Teardown();
	cout << "Launcher Teardown " << endl;
	TeardownWorld();
}


void MLauncher::Update()
{
	static auto& Timer = MTimer::Get();
	const float DeltaTime = Timer.GetDelta();

	assert(World && " World was nullptr ");

	World->Tick(DeltaTime);
}

void MLauncher::Serialize()
{
	if (World)
	{
		World->Serialize();
	}
}

void MLauncher::GenerateWorld()
{
	TeardownWorld();

	World = new MWorld();
	World->Init();
}

void MLauncher::TeardownWorld()
{
	if (World)
	{
		World->Teardown();
		delete World;
	}
}
