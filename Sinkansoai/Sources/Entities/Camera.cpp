#include "Camera.h"
#include "../Input.h"
#include <DirectXMath.h>

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

	if (MInput::Get().IsPressed('Z'))
	{
		this->Transform.Rotation.y -= Move * 0.08f;
	}


	if (MInput::Get().IsPressed('X'))
	{
		this->Transform.Rotation.y += Move * 0.08f;
	}

	const DirectX::XMMATRIX RotationOnly = DirectX::XMMatrixRotationRollPitchYaw(
		DirectX::XMConvertToRadians(this->Transform.Rotation.x),
		DirectX::XMConvertToRadians(this->Transform.Rotation.y),
		DirectX::XMConvertToRadians(this->Transform.Rotation.z));

	const DirectX::XMVECTOR Forward = DirectX::XMVector3Normalize(DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0.f, 0.f, 1.f, 0.f), RotationOnly));
	const DirectX::XMVECTOR Right = DirectX::XMVector3Normalize(DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1.f, 0.f, 0.f, 0.f), RotationOnly));
	const DirectX::XMVECTOR Up = DirectX::XMVector3Normalize(DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f), RotationOnly));

	DirectX::XMVECTOR Movement = DirectX::XMVectorZero();

	// Keyboard Input
	if (MInput::Get().IsPressed('W'))
	{
		Movement = DirectX::XMVectorAdd(Movement, DirectX::XMVectorScale(Forward, Move));
	}

	if (MInput::Get().IsPressed('S'))
	{
		Movement = DirectX::XMVectorAdd(Movement, DirectX::XMVectorScale(Forward, -Move));
	}

	if (MInput::Get().IsPressed('A'))
	{
		Movement = DirectX::XMVectorAdd(Movement, DirectX::XMVectorScale(Right, -Move));
	}

	if (MInput::Get().IsPressed('D'))
	{
		Movement = DirectX::XMVectorAdd(Movement, DirectX::XMVectorScale(Right, Move));
	}

	if (MInput::Get().IsPressed('U'))
	{
		Movement = DirectX::XMVectorAdd(Movement, DirectX::XMVectorScale(Up, Move));
	}

	if (MInput::Get().IsPressed('I'))
	{
		Movement = DirectX::XMVectorAdd(Movement, DirectX::XMVectorScale(Up, -Move));
	}

	if (!DirectX::XMVector3Equal(Movement, DirectX::XMVectorZero()))
	{
		DirectX::XMVECTOR Position = DirectX::XMVectorSet(this->Transform.Position.x, this->Transform.Position.y, this->Transform.Position.z, 1.f);
		Position = DirectX::XMVectorAdd(Position, Movement);
		DirectX::XMStoreFloat4(&this->Transform.Position, Position);
		this->Transform.Position.w = 1.f;
	}
}
