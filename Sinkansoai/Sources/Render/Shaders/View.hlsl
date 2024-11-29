#ifndef VIEW_HLSL
#define VIEW_HLSL

cbuffer View : register(b0, space0)
{
    float4x4 ViewToWorldMatrix;
    float4x4 WorldToViewMatrix;
    float4x4 ProjMatrix;
    float4x4 InvProjMatrix;
    float4x4 WorldToClip;

    float DeltaTime;
    float WorldTime;
    uint2 ViewRect;
	uint DebugInput;

	float3 ViewTranslation;
};

struct Directional
{
	// Utilize w channel for data compaction
	float4 Direction;
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
};

cbuffer LightData : register(b1, space0)
{
	Directional DirectionalLight;
};

float3 CalcDirectionalLight(Directional Light, float3 N, float3 V, float3 BaseColor)
{
    float3 L = normalize(-Light.Direction.xyz);

    float NoL = max(dot(N, L), 0.0);
    float3 R = reflect(-L, N);
    float VoR = max(dot(V, R), 0.0);
		
    float3 Diffuse  = Light.Diffuse.xyz  * NoL * BaseColor * rcp(3.141592);
    float3 Specular = 0;//Light.Specular.xyz * VoR * (1 - NoL) * 0.1f;
    return (Diffuse + Specular);
}  


float3 GetDebugColor(uint ColorId)
{
	switch (ColorId)
	{
	case 0: return float3(0.1f, 0.1f, 0.2f); // Dark Blue
	case 1: return float3(0.1f, 1.0f, 0.1f); // Green
	case 2: return float3(1.0f, 0.1f, 0.1f); // Red
	case 3: return float3(0.6f, 0.4f, 0.1f); // Brown
	case 4: return float3(0.1f, 0.4f, 0.4f); // Brown
	case 5: return float3(0.2f, 0.6f, 0.5f); // Cyan
	case 6: return float3(0.2f, 0.2f, 0.8f); // Cyan
	case 7: return float3(0.6f, 0.1f, 0.5f);
	case 8: return float3(0.7f, 1.0f, 1.0f);
	case 9: return float3(0.3f, 1.0f, 1.0f);
	case 10: return float3(0.5f, 0.5f, 1.0f);
	case 11: return float3(1.0f, 0.8f, 0.3f);
	case 12: return float3(1.0f, 1.0f, 0.0f);
	default: return float3(1.0f, 1.0f, 1.0f); // White
	}
}

float CalcSceneDepth( float DeviceZ )
{
	return ProjMatrix[3][2] / (DeviceZ - ProjMatrix[2][2]);
}


#endif
