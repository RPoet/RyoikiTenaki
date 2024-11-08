#include "Engine.h"
#include "Tasks/TaskSystem.h"
#include "Module.h"
#include "RenderBackend/RenderBackendCommon.h"

#include "Timer.h"
#include "Input.h"

#include "Algorithm.h"


void MEngine::Init()
{
	for (int32 i = 0; i < Modules.size(); ++i)
	{
		//Modules[i]->PrintName();
		Modules[i]->Init();
	}

	Sort(Modules.begin(), Modules.end(), [](const MModuleBase* A, const MModuleBase* B)
	{
		return A->GetPriority() < B->GetPriority();
	});

	// Late Intialization to wait for dependency module initialization
	InitBackend(TEXT("D3D12"));

	bRun = true;
}


void MEngine::Exit()
{
	for (int32 i = 0; i < Modules.size(); ++i)
	{
		Modules[i]->Teardown();
	}

	TeardownBackend();
}


void MEngine::Loop()
{
	for (int32 i = 0; i < Modules.size(); ++i)
	{
		Modules[i]->Update();
	}

	DeltaTime = MTimer::Get().GetDelta();


	static float AccumulatedTime = 0;
	AccumulatedTime+=DeltaTime;

	//if (MInput::Get().IsPressed('Q'))
	//{
	//	bRun = false;
	//}


	MTaskSystem::Get().LaunchTasks();
}


void MEngine::RegisterModule(MModuleBase* Module)
{
	Modules.push_back(Module);
}
