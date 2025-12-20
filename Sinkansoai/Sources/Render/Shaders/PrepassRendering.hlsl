#include "View.hlsl"

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
};

PSInput VSMain(float3 Position : POSITION
			 , float2 UV : TEXCOORD)
{
	const float3 WorldPosition = Position.xyz;

	PSInput Result;
	Result.Position = mul( WorldToClip, float4(WorldPosition, 1) );
	Result.UV = UV;
	return Result;
}

void PSMain(PSInput In)
{ 
	const uint Index = MaterialIndex;	
	const uint TexutreBase = MaterialIndex * 2;
	const float4 DiffuseColor = MaterialTextures[TexutreBase + 0].Sample(Sampler, In.UV) * float4(1, 1, 1, 1);
	const bool bValid = DiffuseColor.a > 0;
	if(!bValid)
	{
		discard;
	}
}
