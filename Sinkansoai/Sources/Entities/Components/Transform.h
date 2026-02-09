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

	// Using SIMDMath types (aliased in Vector.h or used directly)
    // Assuming float3/float4 map to SIMDMath::Vector3/4 or we use them directly.
    // Given my update to Vector.h, float3 should be SIMDMath::Vector3.
    
	float3 TransformPoint(float3 V)
	{
        // Transform point (w=1)
        return SIMDMath::TransformPoint(V, ToMatrix());
	}

	float3 TransformVector(float3 V)
	{
        // Transform vector (w=0)
        return SIMDMath::TransformVector(V, ToMatrix());
	}

	float3 GetDirection() const
	{
		return const_cast<MTransform*>(this)->TransformVector({0, 0, 1});
	}

	float4x4& ToMatrix()
	{
        if (bIsDirty)
        {
		    auto RotationM = SIMDMath::Matrix4x4::RotationRollPitchYaw(
                Rotation.x * SIMDMath::DegToRad, 
                Rotation.y * SIMDMath::DegToRad, 
                Rotation.z * SIMDMath::DegToRad
            );
		    auto ScaleM = SIMDMath::Matrix4x4::Scale(Scale.x, Scale.y, Scale.z);
		    auto TranslationM = SIMDMath::Matrix4x4::Translation(Position.x, Position.y, Position.z);

            // Correct order: Scale * Rotation * Translation
		    LocalToWorld = ScaleM * RotationM * TranslationM;
            bIsDirty = false;
        }
		return LocalToWorld;
	}

	float4x4 GetLocalToWorld()
	{
		return ToMatrix();
	}
};
