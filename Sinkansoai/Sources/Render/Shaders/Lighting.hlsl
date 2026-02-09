#ifndef LOCAL_LIGHT
#include "ScreenPass.hlsl"
#endif
#include "View.hlsl"
#include "SceneTextures.hlsl"

cbuffer LocalLightData : register(b0, space3)
{
	uint LightIndex;
};

#if LOCAL_LIGHT

struct PSInput
{
	float4 Position : SV_POSITION;
	uint InstanceId : SV_INSTANCEID;
};

PSInput VSMain(float3 Position : POSITION
			 , uint InstanceId : SV_INSTANCEID)
{
	const float Radius = PointLights[InstanceId].WorldPositionAndIntensity.w;	
	const float3 WorldPosition = Position.xyz * Radius + PointLights[InstanceId].WorldPositionAndIntensity.xyz;

	PSInput Result;
	Result.Position = mul( WorldToClip, float4(WorldPosition, 1) );
	Result.InstanceId = InstanceId;
	return Result;
}
#endif

void PSMain( in PSInput In,
			out float4 OutSceneColor  : SV_TARGET0 )
{ 
	float3 SVPosition = In.Position.xyz;
	float2 BufferUV = (In.Position.xy + 0.5f) * rcp(ViewRect);
	uint2 PixelPosition = SVPosition.xy;

	float4 BaseColor = BaseColorTexture[PixelPosition];
	float4 GBuffer0 = WorldNormalTexture[PixelPosition];
	float4 WorldPosition = MaterialTexture[PixelPosition].xyzw;
	float3 Emissive = SceneColor[PixelPosition].xyz;
	float3 ViewPosition = ViewTranslation.xyz - WorldPosition.xyz;

	float3 WorldNormal = normalize( GBuffer0.xyz * 2.0f - 1.0f );

#if LOCAL_LIGHT
	const float Radius = PointLights[In.InstanceId].WorldPositionAndIntensity.w;	

	LightContext Context = (LightContext)0;
	Context.LightWorldPosition = PointLights[In.InstanceId].WorldPositionAndIntensity.xyz;
	Context.WorldPosition =  WorldPosition.xyz;
	Context.Color = PointLights[In.InstanceId].Color.xyz;
	Context.Intensity = 10.0f;
	Context.StartFalloff = 0;
	Context.EndFalloff = Radius;
	float3 Lighting = CalcLightPointLightEnergy(Context, WorldNormal, normalize(ViewPosition), BaseColor.xyz);

#else
	float3 Lighting = CalcDirectionalLight( DirectionalLight, WorldNormal, normalize(ViewPosition), BaseColor.xyz ); 
#endif
	OutSceneColor = float4( Lighting + Emissive, 1);
}
