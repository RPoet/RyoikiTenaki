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
Texture2D DiffuseMaterials[25] : register(t1);

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};


PSInput VSMain(float3 Position : POSITION, float2 UV : TEXCOORD)
{
    PSInput Result;
    Result.Position = mul( WorldToClip, float4(Position.xyz, 1) );
    Result.UV = UV;
    Result.Color = float4(UV, 0, 1);
    return Result;
}

float4 PSMain(PSInput In) : SV_TARGET
{ 
    const float DepthFade = In.Position.z * 100; 
    const uint Index = MaterialIndex;
    const float3 Color = GetDebugColor(Index % 13);
    const float4 DiffuseColor = DiffuseMaterials[MaterialIndex].Sample(Sampler, In.UV) * float4(1, 1, 1, 1);
#if 0
    return float4(In.UV, 0, 1);
#else
    return float4(DiffuseColor.xyz, 1);
 #endif
}
