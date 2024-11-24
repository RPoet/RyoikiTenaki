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
    float3 WorldPosition : POSITION;
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
    PSInput Result;
    Result.Position = mul( WorldToClip, float4(Position.xyz, 1) );
    Result.UV = UV;
    Result.WorldPosition = Position.xyz;
    Result.Normal = Normal.xyz;
    Result.Tangent = Tangent.xyz;
    Result.Bitangent = Bitangent.xyz;

    Result.Color = float4(UV, 0, 1);
    return Result;
}

void PSMain(PSInput In
, out float4 Color0  : SV_TARGET0
, out float4 Color1  : SV_TARGET1 // base
, out float4 Color2  : SV_TARGET2 // world normal
, out float4 Color3  : SV_TARGET3 // material
)
{ 
    const float DepthFade = In.Position.z * 100; 
    const uint Index = MaterialIndex;
    const float3 Color = GetDebugColor(Index % 13);
    
    const uint TexutreBase = MaterialIndex * 2;
    const float4 DiffuseColor = MaterialTextures[TexutreBase + 0].Sample(Sampler, In.UV) * float4(1, 1, 1, 1);
    const float3 LocalNormal = MaterialTextures[TexutreBase + 1].Sample(Sampler, In.UV).xyz * 2 - 1;
    
    const float3x3 TangentToWorld = float3x3(
        In.Tangent,
        In.Bitangent,
        In.Normal
    );

    float3 WorldNormal = normalize( mul ( TangentToWorld, LocalNormal ) ) * 0.5f + 0.5f;

    Color0 = float4(0, 0, 0, 1);
    Color1 = float4(DiffuseColor.xyz, 1);
    Color2 = float4(WorldNormal.xyz, 1);
    Color3 = float4(1, 1, 1, 1);
}
