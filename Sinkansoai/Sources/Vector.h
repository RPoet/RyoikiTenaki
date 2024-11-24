#pragma once
#include "PlatformDefinitions.h"

#include <DirectXMath.h>

using float2 = DirectX::XMFLOAT2;
using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;

using int2 = DirectX::XMINT2;
using int3 = DirectX::XMINT3;
using int4 = DirectX::XMINT4;

using uint2 = DirectX::XMUINT2;
using uint3 = DirectX::XMUINT3;
using uint4 = DirectX::XMUINT4;

using XMVECTOR = DirectX::XMVECTOR;

#include <cmath>

float2 operator-(const float2& RHS, const float2& LHS)
{
	return float2
	{
		RHS.x - LHS.x,
		RHS.y - LHS.y,
	};
}

float2 operator*(const float2& RHS, const float2& LHS)
{
	return float2
	{
		RHS.x * LHS.x,
		RHS.y * LHS.y,
	};
}

float2 operator/(const float2& RHS, const float2& LHS)
{
	return float2
	{
		RHS.x / LHS.x,
		RHS.y / LHS.y,
	};
}

float3 operator-(const float3& RHS, const float3& LHS)
{
	return float3
	{
		RHS.x - LHS.x,
		RHS.y - LHS.y,
		RHS.z - LHS.z,
	};
}

float3 operator*(const float3& RHS, const float3& LHS)
{
	return float3
	{
		RHS.x * LHS.x,
		RHS.y * LHS.y,
		RHS.z * LHS.z,
	};
}

float3 operator/(const float3& RHS, const float3& LHS)
{
	return float3
	{
		RHS.x / LHS.x,
		RHS.y / LHS.y,
		RHS.z / LHS.z,
	};
}

float4 operator-(const float4& RHS, const float4& LHS)
{
	return float4
	{
		RHS.x - LHS.x,
		RHS.y - LHS.y,
		RHS.z - LHS.z,
		RHS.w - LHS.w,
	};
}


float4 operator/(const float4& RHS, const float4& LHS)
{
	return float4
	{
		RHS.x / LHS.x,
		RHS.y / LHS.y,
		RHS.z / LHS.z,
		RHS.w / LHS.w,
	};
}

float4 operator*(const float4& RHS, const float4& LHS)
{
	return float4
	{
		RHS.x * LHS.x,
		RHS.y * LHS.y,
		RHS.z * LHS.z,
		RHS.w * LHS.w,
	};
}


float Dot(const float3& RHS, const float3& LHS)
{
	return RHS.x * LHS.x
		 + RHS.y * LHS.y
		 + RHS.z * LHS.z;
}

float Dot(const float4& RHS, const float4& LHS)
{
	return RHS.x * LHS.x
		+ RHS.y * LHS.y
		+ RHS.z * LHS.z
		+ RHS.w * LHS.w;
}

float3 Normalize(const float3& Vector)
{
	const float Distance = floats::Distance(Vector);
	if (Distance != 0)
	{
		return Vector / float3(Distance, Distance, Distance);
	}

	return float3(0, 0, 0);
}

float4 Normalize(const float4& Vector)
{
	const float Distance = floats::Distance(Vector);
	if (Distance != 0)
	{
		return Vector / float4(Distance, Distance, Distance, Distance);
	}

	return float4(0, 0, 0, 0);
}

namespace floats
{
	template<class T>
	float SqrDistance(const T& Vector)
	{
		return Dot(Vector, Vector);
	}

	template<class T>
	float Distance(const T& Vector)
	{	
		return std::sqrt(SqrDistance(Vector));
	}
};
