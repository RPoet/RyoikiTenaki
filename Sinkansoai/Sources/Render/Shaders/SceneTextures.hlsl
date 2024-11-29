#ifndef SCENE_TEXTURES_HLSL
#define SCENE_TEXTURES_HLSL

Texture2D<float>  SceneDepthTexture  : register(t0);
Texture2D<float4> SceneColor         : register(t1);
Texture2D<float4> BaseColorTexture   : register(t2);
Texture2D<float4> WorldNormalTexture : register(t3);
Texture2D<float4> MaterialTexture    : register(t4);

struct GBuffer
{
	float Depth;
	float4 SceneColor;
	float4 BaseColor;
	float4 WorldNormal;
	float4 MaterialTexture;
};

#endif
