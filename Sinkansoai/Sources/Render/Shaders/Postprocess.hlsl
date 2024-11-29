#include "ScreenPass.hlsl"
#include "SceneTextures.hlsl"

float4 PSMain(PSInput In) : SV_TARGET
{ 
    float2 SVPosition = In.Position.xy;
    float2 BufferUV = (In.Position.xy + 0.5f)* rcp(ViewRect);

    uint2 PixelPosition = SVPosition.xy;
    return SceneColor[PixelPosition];
}
