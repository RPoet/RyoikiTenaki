#include "Camera.h"
#include "../Input.h"

void MCamera::Register()
{
	Super::Register();
}

void MCamera::Destroy()
{
	Super::Destroy();
}

void MCamera::Tick(float DeltaTime)
{

	// Keyboard Input
	if (MInput::Get().IsPressed('W'))
	{
		cout << " W is pressed " << endl;
	}

	if (MInput::Get().IsPressed('A'))
	{
		cout << " A is pressed " << endl;
	}

	if (MInput::Get().IsPressed('S'))
	{
		cout << " S is pressed " << endl;
	}

	if (MInput::Get().IsPressed('D'))
	{
		cout << " D is pressed " << endl;
	}


	// Rotation movement
	//MInput::Get().IsPressed('Q');
	//MInput::Get().IsPressed('E');
}
