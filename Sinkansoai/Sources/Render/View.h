#pragma once
#include "../PlatformDefinitions.h"

struct RViewContext
{
	XMMATRIX LocalToWorld;
	float Fov{};
	float MinZ{};
	uint2 ViewRect{};
};


__declspec(align(16u)) class RViewMatrices
{
public:
	XMMATRIX ViewToWorldMatrix;
	XMMATRIX WorldToViewMatrix;
	XMMATRIX ProjMatrix;
	XMMATRIX WorldToClip;


	float DeltaTime;
	float WorldTime;
	float Offset;
};
