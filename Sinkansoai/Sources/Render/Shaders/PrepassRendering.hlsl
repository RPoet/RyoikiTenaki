#include "View.hlsl"

struct PSInput
{
    float4 Position : SV_POSITION;
};


PSInput VSMain(float3 Position : POSITION)
{
    PSInput Result;
    Result.Position = mul( WorldToClip, float4(Position.xyz, 1) );
    return Result;
}
