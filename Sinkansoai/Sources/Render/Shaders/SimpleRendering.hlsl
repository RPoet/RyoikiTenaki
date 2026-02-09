#ifndef USE_GBUFFER
#define USE_GBUFFER 0
#endif

#ifndef NUM_MATERIAL_TEXTURES
#define NUM_MATERIAL_TEXTURES 50
#endif

#ifndef NUM_UV_CHANNELS
#define NUM_UV_CHANNELS 1
#endif

#ifndef MATERIAL_TEXTURE_STRIDE
#define MATERIAL_TEXTURE_STRIDE 2
#endif

#define TEX_DIFFUSE   0
#define TEX_NORMAL    1
#define TEX_SPECULAR  2
#define TEX_SHININESS 3
#define TEX_AMBIENT   4
#define TEX_EMISSIVE  5
#define TEX_REFLECTION 6

#define MATFLAG_HAS_METALLIC_ROUGHNESS 1
#define MATFLAG_HAS_SPECULAR          2
#define MATFLAG_HAS_SHININESS         4
#define MATFLAG_HAS_AMBIENT           8
#define MATFLAG_HAS_EMISSIVE          16

#define FOWARD_LIGHTING !USE_GBUFFER

#include "View.hlsl"

cbuffer ColorData : register(b0, space1)
{
	float3 Tint;
};

cbuffer MaterialData : register(b0, space2)
{
	uint MaterialIndex;
	uint BaseColorUVIndex;
	uint NormalUVIndex;
	uint MetallicRoughnessUVIndex;
	uint AmbientUVIndex;
	uint EmissiveUVIndex;
	uint ShininessUVIndex;
	uint MaterialFlags;
};

SamplerState Sampler : register(s0);
Texture2D DefaultTexture : register(t0);
Texture2D MaterialTextures[NUM_MATERIAL_TEXTURES] : register(t1);

struct PSInput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
#if NUM_UV_CHANNELS > 1
	float2 UV1 : TEXCOORD1;
#endif
	float3 ScreenVector : POSITION0;
	float3 WorldPosition : POSITION1;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
	float4 Color : COLOR;
};


PSInput VSMain(float3 Position : POSITION
			 , float2 UV : TEXCOORD0
#if NUM_UV_CHANNELS > 1
			 , float2 UV1 : TEXCOORD1
#endif
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
#if NUM_UV_CHANNELS > 1
	Result.UV1 = UV1;
#endif
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

float2 SelectUV(uint UVIndex, float2 UV0, float2 UV1)
{
#if NUM_UV_CHANNELS > 1
	return (UVIndex == 1) ? UV1 : UV0;
#else
	return UV0;
#endif
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
	
	const uint TexutreBase = MaterialIndex * MATERIAL_TEXTURE_STRIDE;
	float2 BaseUV = SelectUV(BaseColorUVIndex, In.UV,
#if NUM_UV_CHANNELS > 1
		In.UV1
#else
		In.UV
#endif
	);
	float2 NormalUV = SelectUV(NormalUVIndex, In.UV,
#if NUM_UV_CHANNELS > 1
		In.UV1
#else
		In.UV
#endif
	);
	float2 MetallicRoughnessUV = SelectUV(MetallicRoughnessUVIndex, In.UV,
#if NUM_UV_CHANNELS > 1
		In.UV1
#else
		In.UV
#endif
	);
	float2 AmbientUV = SelectUV(AmbientUVIndex, In.UV,
#if NUM_UV_CHANNELS > 1
		In.UV1
#else
		In.UV
#endif
	);
	float2 EmissiveUV = SelectUV(EmissiveUVIndex, In.UV,
#if NUM_UV_CHANNELS > 1
		In.UV1
#else
		In.UV
#endif
	);
	float2 ShininessUV = SelectUV(ShininessUVIndex, In.UV,
#if NUM_UV_CHANNELS > 1
		In.UV1
#else
		In.UV
#endif
	);

	const float4 DiffuseColor = MaterialTextures[TexutreBase + TEX_DIFFUSE].Sample(Sampler, BaseUV);
	const float4 Normal = MaterialTextures[TexutreBase + TEX_NORMAL].Sample(Sampler, NormalUV);

	float metallic = 0.0f;
	float roughness = 1.0f;
	if ((MaterialFlags & MATFLAG_HAS_METALLIC_ROUGHNESS) != 0)
	{
		const float4 MetallicRoughness = MaterialTextures[TexutreBase + TEX_SPECULAR].Sample(Sampler, MetallicRoughnessUV);
		metallic = saturate(MetallicRoughness.b);
		roughness = saturate(MetallicRoughness.g);
	}
	else
	{
		if ((MaterialFlags & MATFLAG_HAS_SPECULAR) != 0)
		{
			metallic = 0.0f;
		}
		if ((MaterialFlags & MATFLAG_HAS_SHININESS) != 0)
		{
			const float4 Shininess = MaterialTextures[TexutreBase + TEX_SHININESS].Sample(Sampler, ShininessUV);
			roughness = saturate(1.0f - Shininess.r);
		}
	}

	float ao = 1.0f;
	if ((MaterialFlags & MATFLAG_HAS_AMBIENT) != 0)
	{
		ao = saturate(MaterialTextures[TexutreBase + TEX_AMBIENT].Sample(Sampler, AmbientUV).r);
	}

	float3 emissive = 0.0f;
	if ((MaterialFlags & MATFLAG_HAS_EMISSIVE) != 0)
	{
		emissive = MaterialTextures[TexutreBase + TEX_EMISSIVE].Sample(Sampler, EmissiveUV).rgb;
	}

	float SpecularStrength = 0.04f;
	float Shininess = 32.0f;
	if ((MaterialFlags & MATFLAG_HAS_METALLIC_ROUGHNESS) != 0)
	{
		SpecularStrength = lerp(0.04f, 1.0f, metallic);
		Shininess = lerp(2.0f, 128.0f, 1.0f - roughness);
	}
	else
	{
		if ((MaterialFlags & MATFLAG_HAS_SPECULAR) != 0)
		{
			const float4 Specular = MaterialTextures[TexutreBase + TEX_SPECULAR].Sample(Sampler, MetallicRoughnessUV);
			SpecularStrength = saturate(Specular.r);
		}
		if ((MaterialFlags & MATFLAG_HAS_SHININESS) != 0)
		{
			const float4 ShininessTex = MaterialTextures[TexutreBase + TEX_SHININESS].Sample(Sampler, ShininessUV);
			Shininess = lerp(2.0f, 128.0f, 1.0f - saturate(ShininessTex.r));
		}
	}

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
	const float3 GBufferBase = DiffuseColor.xyz * ao;
	Color0 = float4(emissive, 1);
	Color1 = float4(GBufferBase, 1);
	Color2 = float4(WorldNormal, 1);
	Color3 = float4(In.WorldPosition, SceneDepth);
#else
	const float3 BaseColorLit = DiffuseColor.xyz * ao;
	float3 Lighting = CalcDirectionalLight( DirectionalLight, WorldNormal.xyz, ViewVector, BaseColorLit, SpecularStrength, Shininess );

	LightContext Context = (LightContext)0;
	Context.WorldPosition =  In.WorldPosition;
	Context.Intensity = 10.0f;
	Context.StartFalloff = 0;
	
	for(uint i = 0; i < NumPointLights; ++i)
	{
		Context.LightWorldPosition = PointLights[i].WorldPositionAndIntensity.xyz;
		Context.EndFalloff = PointLights[i].WorldPositionAndIntensity.w;
		Context.Color = PointLights[i].Color.xyz;

		Lighting += CalcLightPointLightEnergy(Context, WorldNormal, ViewVector, BaseColorLit, SpecularStrength, Shininess);
	}

	Color0 = float4(Lighting + emissive, 1);
#endif
}
