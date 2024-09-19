#include "Engine.h"
#include "Module.h"
#include "RenderBackendCommon.h"

#include "Timer.h"
#include "Input.h"

void MEngine::Init()
{
	InitBackend(TEXT( "D3D12" ));

	for (int32 i = 0; i < Modules.size(); ++i)
	{
		Modules[i]->Init();
	}
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

	cout << "Time : " << AccumulatedTime << endl;

	for (int i = 0; i < 256; ++i)
	{
		if (MInput::Get().IsPressed(i))
		{
			cout << (char)i << " is pressed " << endl;
		}
	}
}

void MEngine::RegisterModule(MModuleBase* Module)
{
	Modules.push_back(Module);
}
