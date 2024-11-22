#include "View.hlsl"

static const float Edge = 3;
static float4 VertexPositions[3] =
{
    float4(1.0, Edge, 0.5, 1.0),
    float4(-Edge, -1.0, 0.5, 1.0),
    float4(1.0, -1.0, 0.5, 1.0)
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

PSInput VSMain(uint VertexId : SV_VertexID)
{
    const float Edge = 3;
    PSInput Result;
    Result.Position = VertexPositions[VertexId];
    Result.Color = clamp(Result.Position, 0, 1);
    return Result;
}


Texture2D<float>  SceneDepthTexture  : register(t0);
Texture2D<float4> SceneColor         : register(t1);
Texture2D<float4> BaseColorTexture   : register(t2);
Texture2D<float4> WorldNormalTexture : register(t3);
Texture2D<float4> MaterialTexture    : register(t4);

float4 PSMain(PSInput In) : SV_TARGET
{ 
    float2 SVPosition = In.Position.xy;
    float2 BufferUV = (In.Position.xy + 0.5f)* rcp(ViewRect);

    uint2 PixelPosition = SVPosition.xy;
    float Depth = SceneDepthTexture[PixelPosition];

    float4 BaseColor = BaseColorTexture[PixelPosition];
    
#if 0
    return float4( Depth * 100, 0, 0, 1 );
#else 
    return float4( BaseColor );
#endif
}
