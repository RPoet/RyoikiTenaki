#include "View.hlsl"

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
};

PSInput VSMain(float3 Position : POSITION
			 , float2 UV : TEXCOORD0
#if NUM_UV_CHANNELS > 1
			 , float2 UV1 : TEXCOORD1
#endif
			 )
{
	const float3 WorldPosition = Position.xyz;

	PSInput Result;
	Result.Position = mul( WorldToClip, float4(WorldPosition, 1) );
	Result.UV = UV;
#if NUM_UV_CHANNELS > 1
	Result.UV1 = UV1;
#endif
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

void PSMain(PSInput In)
{ 
	const uint Index = MaterialIndex;	
	const uint TexutreBase = MaterialIndex * MATERIAL_TEXTURE_STRIDE;
	float2 BaseUV = SelectUV(BaseColorUVIndex, In.UV,
#if NUM_UV_CHANNELS > 1
		In.UV1
#else
		In.UV
#endif
	);
	const float4 DiffuseColor = MaterialTextures[TexutreBase + TEX_DIFFUSE].Sample(Sampler, BaseUV) * float4(1, 1, 1, 1);
	const bool bValid = DiffuseColor.a > 0;
	if(!bValid)
	{
		discard;
	}
}
