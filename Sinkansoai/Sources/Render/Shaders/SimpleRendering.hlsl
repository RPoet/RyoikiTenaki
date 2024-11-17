

cbuffer View : register(b0, space0)
{
    float4x4 ViewToWorldMatrix;
    float4x4 WorldToViewMatrix;
    float4x4 ProjMatrix;
    float4x4 WorldToClip;

    float DeltaTime;
    float WorldTime;
    float Offset;
    float Pad;
};

cbuffer SectionData : register(b0, space1)
{
    uint SectionIndex;
};

cbuffer MaterialData : register(b0, space2)
{
    uint MaterialIndex;
};

SamplerState Sampler : register(s0);

Texture2D DefaultTexture : register(t0);
Texture2D DiffuseMaterials[25] : register(t1);

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
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

PSInput VSMain(float3 Position : POSITION, float2 UV : TEXCOORD)
{
    PSInput Result;
    Result.Position = mul( WorldToClip, float4(Position.xyz, 1) );
    Result.UV = UV;
    Result.Color = float4(UV, 0, 1);
    return Result;
}


float4 PSMain(PSInput In) : SV_TARGET
{ 
    const float DepthFade = In.Position.z * 100;
    
    const uint Index = MaterialIndex;
    const float3 Color = GetDebugColor(Index % 13);

    const float4 TextureColor = DefaultTexture.Sample(Sampler, In.UV);
    const float4 DiffuseColor = DiffuseMaterials[MaterialIndex].Sample(Sampler, In.UV);
#if 0
    return float4(In.UV, 0, 1);
#else
    return float4(DiffuseColor.xyz, 1);
 #endif
}
