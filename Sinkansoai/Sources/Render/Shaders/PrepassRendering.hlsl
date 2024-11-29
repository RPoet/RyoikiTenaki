#include "View.hlsl"

struct PSInput
{
    float4 Position : SV_POSITION;
};

PSInput VSMain(float3 Position : POSITION)
{
    const float3 WorldPosition = Position.xyz;

    PSInput Result;
    Result.Position = mul( WorldToClip, float4(WorldPosition, 1) );
    return Result;
}
