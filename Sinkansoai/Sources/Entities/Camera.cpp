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
	const float3 FocusPoint = float3(0, 0, 0);

	const float Move = DeltaTime * 500;
	// Keyboard Input
	if (MInput::Get().IsPressed('W'))
	{
		this->Transform.Position.z += Move;
	}

	if (MInput::Get().IsPressed('S'))
	{
		this->Transform.Position.z -= Move;
	}

	if (MInput::Get().IsPressed('A'))
	{
		this->Transform.Position.x -= Move;
	}

	if (MInput::Get().IsPressed('D'))
	{
		this->Transform.Position.x += Move;
	}

	if (MInput::Get().IsPressed('U'))
	{
		this->Transform.Position.y += Move;
	}

	if (MInput::Get().IsPressed('I'))
	{
		this->Transform.Position.y -= Move;
	}



	if (MInput::Get().IsPressed('Z'))
	{
		this->Transform.Rotation.y -= Move * 0.01f;
	}


	if (MInput::Get().IsPressed('X'))
	{
		this->Transform.Rotation.y += Move * 0.01f;
	}

	//cout << this->Transform.Position.x << " " << this->Transform.Position.y << " " << this->Transform.Position.z << endl;

	// Rotation movement
	//MInput::Get().IsPressed('Q');
	//MInput::Get().IsPressed('E');
}
