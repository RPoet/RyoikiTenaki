#ifndef USE_GBUFFER
#define USE_GBUFFER 0
#endif

#define FOWARD_LIGHTING !USE_GBUFFER

#include "View.hlsl"

cbuffer ColorData : register(b0, space1)
{
	float3 Tint;
};

cbuffer MaterialData : register(b0, space2)
{
	uint MaterialIndex;
};

SamplerState Sampler : register(s0);
Texture2D DefaultTexture : register(t0);
Texture2D MaterialTextures[50] : register(t1);

struct PSInput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD;
	float3 ScreenVector : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
	float4 Color : COLOR;
};


PSInput VSMain(float3 Position : POSITION
			 , float2 UV : TEXCOORD
			 , float3 Normal : NORMAL
			 , float3 Tangent : TANGENT
			 , float3 Bitangent : BITANGENT      
			 )
{
	const float3 WorldPosition = Position.xyz;

	PSInput Result;
	Result.Position = mul( WorldToClip, float4(WorldPosition, 1) );
	Result.UV = UV;
	Result.ScreenVector = normalize( mul( WorldToViewMatrix, float4(WorldPosition, 1) ) ).xyz;
	Result.Normal = normalize(Normal.xyz);
	Result.Tangent = normalize(Tangent.xyz);
	Result.Bitangent = normalize(Bitangent.xyz);

	Result.Color = float4(UV, 0, 1);
	return Result;
}

void PSMain(PSInput In
, out float4 Color0  : SV_TARGET0 // Scene Color
#if USE_GBUFFER
, out float4 Color1  : SV_TARGET1 // Base Color
, out float4 Color2  : SV_TARGET2 // World Normal
, out float4 Color3  : SV_TARGET3 // Material
#endif
)
{
	const uint Index = MaterialIndex;
	const float3 Color = GetDebugColor(Index % 13);
	
	const uint TexutreBase = MaterialIndex * 2;
	const float4 DiffuseColor = MaterialTextures[TexutreBase + 0].Sample(Sampler, In.UV);
	const float4 Normal = MaterialTextures[TexutreBase + 1].Sample(Sampler, In.UV);

	float3 ViewVector = In.ScreenVector;
	float3 WorldNormal = In.Normal;
	if(dot(Normal, Normal) > 0)
	{
		const float3 LocalNormal = Normal.xyz * 2 - 1;
	
		const float3x3 TangentBasis = float3x3(
			In.Tangent,
			In.Bitangent,
			In.Normal
		);

		WorldNormal = mul( LocalNormal, TangentBasis );
	}

#if USE_GBUFFER	
	WorldNormal = normalize( WorldNormal ) * 0.5f + 0.5f;

	Color0 = float4(0, 0, 0, 1);
	Color1 = float4(DiffuseColor.xyz, 1);
	Color2 = float4(WorldNormal, 1);
	Color3 = float4(In.Normal * 0.5f + 0.5f, 1);
#else
	float3 Lighting = CalcDirectionalLight( DirectionalLight, WorldNormal.xyz, ViewVector, DiffuseColor.xyz );
	Color0 = float4(Lighting, 1);
#endif
}
