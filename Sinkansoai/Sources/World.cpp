#include "World.h"
#include "Scene.h"
#include "Tasks/TaskSystem.h"


/*
* Example Usage of the render system command.
* 
	MTaskSystem::Get().AddRenderCommand(TEXT("Scene Init Command"),
	[](RRenderCommandList&)
	{
		cout << "Scene Init Command" << endl;
	});
*/
void MWorld::Init()
{
	cout << "World Init" << endl;

	Scene = new RScene();


	// Register Camera Entity after wrapping viewport
	// Use viewport system, Viewport will works soley
	Camera.SetExternalObject(true);
	ObjectSystem.RegisterEntity(&Camera);
}


void MWorld::Teardown()
{
	ObjectSystem.Teardown();

	cout << "World Teardown" << endl;

	if (Scene)
	{
		delete Scene;
	}
}


void MWorld::Tick(float DeltaTime)
{
	// Main loop
	ObjectSystem.Tick(DeltaTime);
}


void MWorld::Serialize()
{
	ObjectSystem.Serialize();
}
