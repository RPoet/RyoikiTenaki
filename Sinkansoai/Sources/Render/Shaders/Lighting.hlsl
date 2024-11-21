#include "View.hlsl"

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

PSInput VSMain(uint VertexId : SV_VertexID)
{
    const float Edge = 3;
    PSInput Result;
    Result.Position = float4(1.0, Edge, 0.5, 1.0);
    
    if ( VertexId == 1 )
        Result.Position = float4(-Edge, -1.0, 0.5, 1.0);
    else if ( VertexId == 2 )
        Result.Position = float4(1.0, -1.0, 0.5, 1.0);
    
    Result.Color = clamp(Result.Position, 0, 1);
    return Result;
}

float4 PSMain(PSInput In) : SV_TARGET
{ 
    float2 SVPosition = In.Position.xy * rcp(ViewRect);


    return float4( SVPosition, 0, 1);
}
