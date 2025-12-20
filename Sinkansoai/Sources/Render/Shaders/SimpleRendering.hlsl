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
	float3 ScreenVector : POSITION0;
	float3 WorldPosition : POSITION1;
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
	Result.WorldPosition = WorldPosition;
	Result.Position = mul( WorldToClip, float4(WorldPosition, 1) );
	Result.UV = UV;
	//Result.ScreenVector = normalize( mul( WorldToViewMatrix, float4(WorldPosition, 1) ) ).xyz;
	
	float4 ScreenVector = mul( InvProjMatrix, Result.Position );
	ScreenVector.xyz /= ScreenVector.w;
	Result.ScreenVector = normalize(ScreenVector.xyz);	

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

	float DeviceZ = In.Position.z;
	float SceneDepth = CalcSceneDepth( DeviceZ );

	float3 ViewPosition = ViewTranslation.xyz - In.WorldPosition.xyz;
	float3 WorldNormal = In.Normal;
	float3 ViewVector = normalize(ViewPosition);
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
	Color3 = float4(In.WorldPosition, SceneDepth);
#else
	float3 Lighting = CalcDirectionalLight( DirectionalLight, WorldNormal.xyz, ViewVector, DiffuseColor.xyz );

	LightContext Context = (LightContext)0;
	Context.WorldPosition =  In.WorldPosition;
	Context.Intensity = 10.0f;
	Context.StartFalloff = 0;
	
	for(uint i = 0; i < NumPointLights; ++i)
	{
		Context.LightWorldPosition = PointLights[i].WorldPositionAndIntensity.xyz;
		Context.EndFalloff = PointLights[i].WorldPositionAndIntensity.w;
		Context.Color = PointLights[i].Color.xyz;

		Lighting += CalcLightPointLightEnergy(Context, WorldNormal, ViewVector, DiffuseColor.xyz);
	}

	Color0 = float4(Lighting, 1);
#endif
}
