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

struct Point
{
	float4 WorldPositionAndIntensity;
	float4 Color;
};

cbuffer LightData : register(b1, space0)
{
	Directional DirectionalLight;
	Point PointLights[500];
	uint NumPointLights;
	uint3 Pad;
};

struct LightContext
{
	float Intensity;
	float3 WorldPosition;
	float3 LightWorldPosition;
	float3 Color;

	float StartFalloff;
	float EndFalloff;	
};

float3 CalcDirectionalLight(Directional Light, float3 N, float3 V, float3 BaseColor)
{
	const float kPi = 3.14159265;
	const float kShininess = 4.0;
	const float LightIntensity = 1;
	const float kEnergyConservation = ( 8.0 + kShininess ) / ( 8.0 * kPi ); 

	float3 L = normalize(-Light.Direction.xyz);
	float3 H = normalize(L + V);

	float NoL = max(dot(N, L), 0.0);
	float3 R = reflect(-L, N);
	float VoR = max(dot(V, R), 0.0);

	float3 Diffuse  = Light.Diffuse.xyz * rcp(kPi)  * NoL * BaseColor;
	float3 Specular = 0;//kEnergyConservation * pow(max(dot(N, H), 0.0), kShininess);
	float3 Ambient = 0;

	return (Diffuse + Specular + Ambient) * LightIntensity;
}

float CalcAttenuation(float d, float Start, float End)
{
    return saturate((End - d) / (End - Start));
}

float3 CalcLightPointLightEnergy(LightContext Context, float3 N, float3 V, float3 BaseColor)
{
	const float kPi = 3.14159265;
	const float kShininess = 4.0;
	const float kEnergyConservation = ( 8.0 + kShininess ) / ( 8.0 * kPi ); 

	float3 LightToPixel = Context.LightWorldPosition - Context.WorldPosition;
	float Length = length(LightToPixel);
	float LightEnergy = CalcAttenuation( Length, Context.StartFalloff, Context.EndFalloff ) * Context.Intensity;
	
	float3 L = LightToPixel / Length;
	float3 H = normalize(L + V);

	float NoL = max(dot(N, L), 0.0);
	float3 R = reflect(-L, N);
	float VoR = max(dot(V, R), 0.0);

	float3 Diffuse  = Context.Color * rcp(kPi) * NoL * BaseColor;
	float3 Specular = 0;//kEnergyConservation * pow(max(dot(N, H), 0.0), kShininess);

	return (Diffuse + Specular) * LightEnergy;
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
