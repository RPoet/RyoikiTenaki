#pragma once
#include "PlatformDefinitions.h"
#include "Math/SIMDMath.h"

// Float vectors - using SIMD implementation
using float2 = SIMDMath::Vector2;
using float3 = SIMDMath::Vector3;
using float4 = SIMDMath::Vector4;

// Int vectors - simple structs to remove DirectXMath dependency
struct int2 { int x, y; };
struct int3 { int x, y, z; };
struct int4 { int x, y, z, w; };

struct uint2 { unsigned int x, y; };
struct uint3 { unsigned int x, y, z; };
struct uint4 { unsigned int x, y, z, w; };

// Matrix compatibility
using XMMATRIX = SIMDMath::Matrix4x4;
using float4x4 = SIMDMath::Matrix4x4; // If not defined elsewhere

inline bool operator==(const float2& A, const float2& B)
{
	return A.x == B.x && A.y == B.y;
}

inline bool operator==(const float3& A, const float3& B)
{
	return A.x == B.x && A.y == B.y && A.z == B.z;
}

inline bool operator==(const float4& A, const float4& B)
{
	return A.x == B.x && A.y == B.y && A.z == B.z && A.w == B.w;
}
