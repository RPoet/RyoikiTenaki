#pragma once
#include "../../SceneGraphSystem/Component.h"

struct MTransform
{
private:
	bool bIsDirty = true;

public:
	float3 Scale{1, 1, 1};
	float3 Rotation{0, 0, 0};
	float4 Position{0, 0, 0, 1};

	XMMATRIX LocalToWorld{};

public:

	MTransform() = default;

	void SetScale(float X, float Y, float Z)
	{
		Scale.x = X;
		Scale.y = Y;
		Scale.z = Y;
		bIsDirty = true;
	}

	// In both left hand and right hand coordinate system
	// X Axis and Y Axis same but Z Axis is different
	
	//  Pitch X Axis, Yaw Y Axis, Roll Z Axis Angle.
	void SetRotation(float Pitch, float Yaw, float Roll)
	{
		Rotation.x = Pitch;
		Rotation.y = Yaw;
		Rotation.z = Roll;
		bIsDirty = true;
	}

	void SetPosition(float X, float Y, float Z)
	{
		Position.x = X;
		Position.y = Y;
		Position.z = Z;
		Position.w = 1;
		bIsDirty = true;
	}

	float3 TransformPoint(float3 V)
	{
		auto Point = DirectX::XMVector3Transform(DirectX::XMVectorSet(V.x, V.y, V.z, 1), ToMatrix());
		float3 Out{};
		DirectX::XMStoreFloat3(&Out, Point);
		return Out;
	}

	float3 TransformVector(float3 V)
	{
		auto Vec = DirectX::XMVector3Transform(DirectX::XMVectorSet(V.x, V.y, V.z, 0), ToMatrix());
		float3 Out{};
		DirectX::XMStoreFloat3(&Out, Vec);
		return Out;
	}

	float3 GetDirection()
	{
		return TransformVector({ 0, 0, 1 });
	}

	XMMATRIX& ToMatrix()
	{
		// fix dirty flag and make all those member variables as private to use dirty flag.
		auto RotationM = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians( Rotation.x ), DirectX::XMConvertToRadians(Rotation.y), DirectX::XMConvertToRadians(Rotation.z));
		auto ScaleM = DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z);
		auto TranslationM = DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z);

		LocalToWorld = TranslationM * ScaleM * RotationM ;

		return LocalToWorld;
	}

	float4x4 GetLocalToWorld()
	{
		float4x4 Out;
		DirectX::XMStoreFloat4x4(&Out, ToMatrix());
		return Out;
	}
};
