#pragma once

#include "../Singleton.h"
#include "../PlatformDefinitions.h"
#include "../Engine/Mesh.h"

class MGeometryGenerator : public Singleton<MGeometryGenerator>
{
public:

	MMesh GenerateBox(float Width, float Height, float Depth)
	{
		MMesh Out;

		float3 Positions[24];

		float w2 = 0.5f * Width;
		float h2 = 0.5f * Height;
		float d2 = 0.5f * Depth;

		Positions[0] = float3( -w2, -h2, -d2 );
		Positions[1] = float3( -w2, +h2, -d2 );
		Positions[2] = float3( +w2, +h2, -d2 );
		Positions[3] = float3( +w2, -h2, -d2 );

		Positions[4] = float3(-w2, -h2, +d2 );
		Positions[5] = float3(+w2, -h2, +d2 );
		Positions[6] = float3(+w2, +h2, +d2 );
		Positions[7] = float3(-w2, +h2, +d2 );

		Positions[8] = float3(-w2, +h2, -d2 );
		Positions[9] = float3(-w2, +h2, +d2 );
		Positions[10] = float3(+w2, +h2, +d2 );
		Positions[11] = float3(+w2, +h2, -d2 );

		Positions[12] = float3(-w2, -h2, -d2 );
		Positions[13] = float3(+w2, -h2, -d2 );
		Positions[14] = float3(+w2, -h2, +d2 );
		Positions[15] = float3(-w2, -h2, +d2 );

		Positions[16] = float3(-w2, -h2, +d2 );
		Positions[17] = float3(-w2, +h2, +d2 );
		Positions[18] = float3(-w2, +h2, -d2 );
		Positions[19] = float3(-w2, -h2, -d2 );

		Positions[20] = float3(+w2, -h2, -d2 );
		Positions[21] = float3(+w2, +h2, -d2 );
		Positions[22] = float3(+w2, +h2, +d2 );
		Positions[23] = float3(+w2, -h2, +d2 );

		Out.RenderData.Positions.assign(&Positions[0], &Positions[24]);

		

		float2 UVs[24];

		UVs[0] = float2(0.0f, 1.0f);
		UVs[1] = float2(0.0f, 0.0f);
		UVs[2] = float2(1.0f, 0.0f);
		UVs[3] = float2(1.0f, 1.0f);

		UVs[4] = float2(1.0f, 1.0f);
		UVs[5] = float2(0.0f, 1.0f);
		UVs[6] = float2(0.0f, 0.0f);
		UVs[7] = float2(1.0f, 0.0f);

		UVs[8] =  float2(0.0f, 1.0f);
		UVs[9] =  float2(0.0f, 0.0f);
		UVs[10] = float2(1.0f, 0.0f);
		UVs[11] = float2(1.0f, 1.0f);

		UVs[12] = float2(1.0f, 1.0f);
		UVs[13] = float2(0.0f, 1.0f);
		UVs[14] = float2(0.0f, 0.0f);
		UVs[15] = float2(1.0f, 0.0f);
		
		UVs[16] = float2(0.0f, 1.0f);
		UVs[17] = float2(0.0f, 0.0f);
		UVs[18] = float2(1.0f, 0.0f);
		UVs[19] = float2(1.0f, 1.0f);

		UVs[20] = float2(0.0f, 1.0f);
		UVs[21] = float2(0.0f, 0.0f);
		UVs[22] = float2(1.0f, 0.0f);
		UVs[23] = float2(1.0f, 1.0f);

		Out.RenderData.GetUVChannel(0).assign(&UVs[0], &UVs[24]);

		float4 Colors[24];

		Colors[0] = float4(1, 0, 0, 1);
		Colors[1] = float4(0, 1, 0, 1);
		Colors[2] = float4(0, 0, 1, 1);
		Colors[3] = float4(1, 1, 1, 1);

		Colors[4] = float4(1, 0, 0, 1);
		Colors[5] = float4(0, 1, 0, 1);
		Colors[6] = float4(0, 0, 1, 1);
		Colors[7] = float4(1, 1, 1, 1);

		Colors[8] =  float4(1, 0, 0, 1);
		Colors[9] =  float4(0, 1, 0, 1);
		Colors[10] = float4(0, 0, 1, 1);
		Colors[11] = float4(1, 1, 1, 1);

		Colors[12] = float4(1, 0, 0, 1);
		Colors[13] = float4(0, 1, 0, 1);
		Colors[14] = float4(0, 0, 1, 1);
		Colors[15] = float4(1, 1, 1, 1);

		Colors[16] = float4(1, 0, 0, 1);
		Colors[17] = float4(0, 1, 0, 1);
		Colors[18] = float4(0, 0, 1, 1);
		Colors[19] = float4(1, 1, 1, 1);

		Colors[20] = float4(1, 0, 0, 1);
		Colors[21] = float4(0, 1, 0, 1);
		Colors[22] = float4(0, 0, 1, 1);
		Colors[23] = float4(1, 1, 1, 1);

		Out.RenderData.Colors.assign(&Colors[0], &Colors[24]);

		uint32 i[36];
		i[0] = 0; i[1] = 1; i[2] = 2;
		i[3] = 0; i[4] = 2; i[5] = 3;


		i[6] = 4; i[7] = 5; i[8] = 6;
		i[9] = 4; i[10] = 6; i[11] = 7;


		i[12] = 8; i[13] = 9; i[14] = 10;
		i[15] = 8; i[16] = 10; i[17] = 11;


		i[18] = 12; i[19] = 13; i[20] = 14;
		i[21] = 12; i[22] = 14; i[23] = 15;


		i[24] = 16; i[25] = 17; i[26] = 18;
		i[27] = 16; i[28] = 18; i[29] = 19;


		i[30] = 20; i[31] = 21; i[32] = 22;
		i[33] = 20; i[34] = 22; i[35] = 23;

		Out.RenderData.Indices.assign(&i[0], &i[36]);

		return Out;
	}
};
