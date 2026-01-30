#include "Engine.h"
#include "Tasks/TaskSystem.h"
#include "Module.h"
#include "RenderBackend/RenderBackendCommon.h"

#include "Timer.h"
#include "Input.h"

#include "Algorithm.h"

#include <cwchar>
#include <cwctype>

static ERenderBackendType SelectBackendFromCmdLine()
{
	String BackendName = TEXT("D3D12");

	if (GStartupParams.lpCmdLine && wcslen(GStartupParams.lpCmdLine) > 0)
	{
		const wchar_t* Token = wcsstr(GStartupParams.lpCmdLine, L"-backend=");
		if (Token)
		{
			Token += wcslen(L"-backend=");

			String Parsed;
			while (*Token && !iswspace(*Token))
			{
				Parsed.push_back(*Token);
				++Token;
			}

			if (!Parsed.empty())
			{
				BackendName = Parsed;
			}
		}
	}

	return BackendTypeFromName(BackendName);
}


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
	InitBackend(SelectBackendFromCmdLine());

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
