#ifndef SCREEN_PASS_HLSL
#define SCREEN_PASS_HLSL
#include "View.hlsl"

static const float Edge = 3;
static float4 VertexPositions[3] =
{
	float4(1.0, 	Edge,  1.0, 1.0),
	float4(-Edge, 	-1.0,  1.0, 1.0),
	float4(1.0, 	-1.0,  1.0, 1.0)
};

struct PSInput
{
	float4 Position : SV_POSITION;
	float3 ScreenVector : POSITION;
	float4 Color : COLOR;
};

PSInput VSMain(uint VertexId : SV_VertexID)
{
	PSInput Result;
	Result.Position = VertexPositions[VertexId];
	float4 ScreenVector = mul( InvProjMatrix, VertexPositions[VertexId] );
	ScreenVector.xyz /= ScreenVector.w;

	Result.ScreenVector = normalize( ScreenVector.xyz );

	Result.Color = Result.Position;
	return Result;
}

#endif
