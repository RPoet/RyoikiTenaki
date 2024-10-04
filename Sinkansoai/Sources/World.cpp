#include "World.h"
#include "Scene.h"
#include "Tasks/TaskSystem.h"

void MWorld::Init()
{
	cout << "World Init" << endl;

	Scene = new RScene();

	MTaskSystem::Get().AddRenderCommand(TEXT("Scene Init Command"),
	[](RRenderCommandList&)
	{
		cout << "Scene Init Command" << endl;
	});
}


void MWorld::Teardown()
{
	cout << "World Teardown" << endl;

	if (Scene)
	{
		delete Scene;
	}
}


void MWorld::Tick(float DeltaTime)
{
	// Main loop


}
