#include "Camera.h"
#include "../Input.h"
#include "../Math/SIMDMath.h"

using namespace SIMDMath;

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
	const float MoveSpeed = DeltaTime * 500;
	const float RotationSpeed = 0.15f;

	if (MInput::Get().IsRightMouseDown())
	{
		int32 DeltaX = MInput::Get().GetMouseDeltaX();
		int32 DeltaY = MInput::Get().GetMouseDeltaY();

		Transform.Rotation.y += DeltaX * RotationSpeed;
		Transform.Rotation.x += DeltaY * RotationSpeed;

		if (Transform.Rotation.x > 89.0f) Transform.Rotation.x = 89.0f;
		if (Transform.Rotation.x < -89.0f) Transform.Rotation.x = -89.0f;
	}

	Matrix4x4 RotMat = GetRotationMatrix();

	Vector3 Forward = TransformVector(Vector3::Forward(), RotMat);
	Vector3 Right = TransformVector(Vector3::Right(), RotMat);
	Vector3 Up = Vector3::Up();

	Vector3 Movement = Vector3::Zero();

	if (MInput::Get().IsPressed('W'))
	{
		Movement += Forward * MoveSpeed;
	}
	if (MInput::Get().IsPressed('S'))
	{
		Movement -= Forward * MoveSpeed;
	}
	if (MInput::Get().IsPressed('A'))
	{
		Movement -= Right * MoveSpeed;
	}
	if (MInput::Get().IsPressed('D'))
	{
		Movement += Right * MoveSpeed;
	}
	if (MInput::Get().IsPressed('E'))
	{
		Movement += Up * MoveSpeed;
	}
	if (MInput::Get().IsPressed('Q'))
	{
		Movement -= Up * MoveSpeed;
	}

	if (Vector3::Dot(Movement, Movement) > 0.0001f)
	{
		Transform.SetPosition(
			Transform.Position.x + Movement.x,
			Transform.Position.y + Movement.y,
			Transform.Position.z + Movement.z);
	}
}
