#pragma once
#include "../../SceneGraphSystem/Component.h"

struct MTransform
{
public:
	float3 Scale{};
	float3 Rotation{};
	float4 Translation{};

public:

	MTransform() = default;

	void SetScale(float X, float Y, float Z)
	{
		Scale.X = X;
		Scale.Y = Y;
		Scale.Z = Z;
	}

	void SetRotation(float X, float Y, float Z)
	{
		Rotation.X = X;
		Rotation.Y = Y;
		Rotation.Z = Z;
	}

	void SetTranslation(float X, float Y, float Z)
	{
		Translation.X = X;
		Translation.Y = Y;
		Translation.Z = Z;
		Translation.W = 1;
	}
};
