#ifndef VIEW_HLSL
#define VIEW_HLSL

cbuffer View : register(b0, space0)
{
    float4x4 ViewToWorldMatrix;
    float4x4 WorldToViewMatrix;
    float4x4 ProjMatrix;
    float4x4 WorldToClip;

    float DeltaTime;
    float WorldTime;
    uint2 ViewRect;
};

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

#endif
