#include "Engine.h"

void MEngine::Init()
{
	Timer.Init();
	Input.Init();
}


void MEngine::Exit()
{
	Input.Exit();
}


void MEngine::Loop()
{
	Timer.Tick();
	Input.Tick();

	DeltaTime = Timer.GetDelta();


	static float AccumulatedTime = 0;
	AccumulatedTime+=DeltaTime;

	cout << "Time : " << AccumulatedTime << endl;

	for (int i = 0; i < 256; ++i)
	{
		if (Input.IsPressed(i))
		{
			cout << (char)i << " is pressed " << endl;
		}
	}

}

