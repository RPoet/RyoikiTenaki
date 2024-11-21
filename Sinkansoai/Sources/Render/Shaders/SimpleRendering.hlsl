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
    float4 Color : COLOR;
};


PSInput VSMain(float3 Position : POSITION
             , float2 UV : TEXCOORD
             , float3 Normal : NORMAL)
{
    PSInput Result;
    Result.Position = mul( WorldToClip, float4(Position.xyz, 1) );
    Result.UV = UV;
    Result.WorldPosition = Position.xyz;
    Result.Normal = Normal.xyz;
    Result.Color = float4(UV, 0, 1);
    return Result;
}

float4 PSMain(PSInput In) : SV_TARGET
{ 
    const float DepthFade = In.Position.z * 100; 
    const uint Index = MaterialIndex;
    const float3 Color = GetDebugColor(Index % 13);

    
    const uint TexutreBase = MaterialIndex * 2;
    const float4 DiffuseColor = MaterialTextures[TexutreBase + 0].Sample(Sampler, In.UV) * float4(1, 1, 1, 1);
    const float4 NormalColor = MaterialTextures[TexutreBase + 1].Sample(Sampler, In.UV) * float4(1, 1, 1, 1);


    const float3 WorldDDX = ddx(In.WorldPosition);
    const float3 WorldDDY = ddy(In.WorldPosition);
    
    const float2 TexDDX = ddx(In.UV); 
    const float2 TexDDY = ddx(In.UV);

    const float3 VertexNormal = normalize(In.Normal);

    float3 Tangent = TexDDY.y * WorldDDX - TexDDX.y * WorldDDY;
    float3 Binormal = TexDDX.x * WorldDDY - TexDDY.x * WorldDDX;
    float3 x = cross( VertexNormal, Tangent );
    Tangent = normalize( cross( x, VertexNormal ) );

    x = cross( Binormal, VertexNormal );
    Binormal = normalize( cross( VertexNormal, x ) ) ;

    //float3 VertexNormal = NormalColor.xyz;

    const float3x3 TBN = transpose( float3x3(
        Tangent,
        Binormal,
        VertexNormal
    ) );

    float3 WorldNormal = normalize( mul (  TBN, NormalColor.xyz  ) );

#if 0
    return float4(In.UV, 0, 1);
#else
    return float4(NormalColor.xyz, 1);
 #endif
}
