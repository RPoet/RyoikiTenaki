#pragma once
#include "../../SceneGraphSystem/Component.h"

struct MTransform
{
private:
	bool bIsDirty = false;

public:
	float3 Scale{};
	float3 Rotation{};
	float4 Position{};

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

	XMMATRIX& ToMatrix()
	{
		if (bIsDirty)
		{
			auto RotationM = DirectX::XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, Rotation.z);
			auto ScaleM = DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z);
			auto TranslationM = DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z);

			LocalToWorld = ScaleM * RotationM * TranslationM;
		}

		return LocalToWorld;
	}

	float4x4 GetLocalToWorld()
	{
		float4x4 Out;
		DirectX::XMStoreFloat4x4(&Out, ToMatrix());
		return Out;
	}
};
