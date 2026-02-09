#pragma once
#include "../Math/SIMDMath.h"

using float4x4 = SIMDMath::Matrix4x4;

// Simple 3x3 struct as placeholder or if actually used
struct float3x3 
{
    float m[3][3];
};

using XMMATRIX = SIMDMath::Matrix4x4;
