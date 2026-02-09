#pragma once
#include "../PlatformDefinitions.h"

struct RViewContext
{
	XMMATRIX LocalToWorld;
	float Fov{};
	float MinZ{};
	uint2 ViewRect{};

	float3 ViewTranslation;
};

// Naming aliases (render-facing types without R-prefix).
using ViewContext = RViewContext;


__declspec(align(16u)) class RViewMatrices
{
public:
	XMMATRIX ViewToWorldMatrix;
	XMMATRIX WorldToViewMatrix;
	XMMATRIX ProjMatrix;
	XMMATRIX InvProjMatrix;
	XMMATRIX WorldToClip;

	float DeltaTime;
	float WorldTime;
	uint2 ViewRect;
	uint32 DebugValue;

	float3 ViewTranslation;
};

using ViewMatrices = RViewMatrices;


__declspec(align(16u)) struct RDirectionalLightData
{
	float4 Direction;
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
};

__declspec(align(16u)) struct RPoint
{
	float4 WorldPositionAndIntensity;
	float4 Color;
};

__declspec(align(16u)) struct RLightData
{
public:
	RDirectionalLightData DirectionalLight;
	RPoint PointLights[500];
	uint32 NumPointLights;
};
