#include "ScreenPass.hlsl"
#include "SceneTextures.hlsl"

void PSMain( in PSInput In,
            out float4 SceneColor  : SV_TARGET0,
            out float4 Debug : SV_TARGET1 )
{ 
    float2 SVPosition = In.Position.xy;
    float2 BufferUV = (In.Position.xy + 0.5f) * rcp(ViewRect);
    uint2 PixelPosition = SVPosition.xy;

    float DeviceZ = SceneDepthTexture[PixelPosition];
    float SceneDepth = CalcSceneDepth( DeviceZ );
    float4 BaseColor = BaseColorTexture[PixelPosition];

    float4 GBuffer0 = WorldNormalTexture[PixelPosition];
    float3 WorldNormal = normalize( GBuffer0.xyz * 2.0f - 1.0f );

    float4 GBuffer1 = MaterialTexture[PixelPosition];
    float3 VertexWorldNormal = normalize( GBuffer1.xyz * 2.0f - 1.0f );

    float3 ViewVector = normalize(In.ScreenVector.xyz);
    float3 ViewPosition = ViewVector * SceneDepth;
    float3 WorldPosition = mul( ViewToWorldMatrix, float4( ViewPosition, 1 ) ).xyz;

    float3 Color = CalcDirectionalLight( DirectionalLight, VertexWorldNormal.xyz, ViewVector, BaseColor.xyz );
  
    SceneColor = 0;
    Debug = 0;
      
    if( DebugInput == 0 )
    {
        SceneColor = float4( Color, 1 );
        Debug = float4(In.ScreenVector.xyz * SceneDepth, SceneDepth);
        return;
    }
    
    if( DebugInput == 1 )
    {
        SceneColor =float4( GBuffer1.xyz, 1 );
        Debug = float4( SceneDepth, SceneDepth, SceneDepth, SceneDepth );
        return;
    }
    
    if( DebugInput == 2 )
    {
        SceneColor = float4( GBuffer0.xyz, 1 );
        Debug = float4( SceneDepth, SceneDepth, SceneDepth, SceneDepth );
        return;
    }
}
