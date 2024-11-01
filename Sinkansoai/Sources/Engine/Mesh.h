#pragma once
#include "../PlatformDefinitions.h"


struct MRenderData
{
	vector< float3 > Positions;
	vector< float4 > Colors;
	vector< float2 > UV0;
	vector< float3 > Normals;
	vector< float3 > Tangents;

	vector< uint32 > Indices;
};

struct MMesh
{
public:
	MRenderData RenderData;


};

