#include "Launcher.h"
#include "Engine.h"
#include "Timer.h"
#include "World.h"


IMPLEMENT_MODULE(MLauncher)

void MLauncher::Init()
{
	Super::Init();
	cout << "Launcher Init " << endl;
	WorldGeneration();
}


void MLauncher::Teardown()
{
	Super::Teardown();
	cout << "Launcher Teardown " << endl;
	WorldTeardown();
}


void MLauncher::Update()
{
	static auto& Timer = MTimer::Get();
	const float DeltaTime = Timer.GetDelta();

	assert(World && " World was nullptr ");

	World->Tick(DeltaTime);
}


void MLauncher::WorldGeneration()
{
	WorldTeardown();

	World = new MWorld();
	World->Init();
}

void MLauncher::WorldTeardown()
{
	if (World)
	{
		World->Teardown();
		delete World;
	}
}
