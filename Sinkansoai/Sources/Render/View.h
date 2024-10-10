#pragma once
#include "../PlatformDefinitions.h"

struct RViewContext
{
	float3 Rotation{};
	float4 Translation{};
	float Fov{};
	float MinZ{};
	uint2 ViewRect{};
};

