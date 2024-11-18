#pragma once
#include "../PlatformDefinitions.h"
#include "../RenderBackend/ResourceBuffer.h"
#include "../RenderBackend/DynamicBuffer.h"
#include "../RenderBackend/Texture.h"

#include "Texture.h"

struct MMaterial
{
public:
	vector< MTexture > Textures;
	bool bValid = false;

	vector< float3 > Colors;
};

struct MRenderData
{
public:
	vector< float3 > Positions;
	vector< float4 > Colors;
	vector< float2 > UV0;
	vector< float3 > Normals;
	vector< float3 > Tangents;

	vector< uint32 > Indices;
};

struct MSectionData
{
public:
	uint32 Start;
	uint32 End;
	uint32 MaterialId;
};

struct MMesh
{
public:
	MRenderData RenderData;
	vector< MSectionData > Sections;
	vector< MMaterial > Materials;
};

struct RMaterial
{
public:
	vector< SharedPtr< RTexture > > Textures;
	vector< float3 > Colors;
	bool bVaild;
};

struct RMesh
{
public:
	RVertexBuffer* PositionVertexBuffer;
	RVertexBuffer* UVVertexBuffer;
	RVertexBuffer* NormalVertexBuffer;
	RVertexBuffer* TangentVertexBuffer;
	RIndexBuffer* IndexBuffer;

	vector< MSectionData > Sections;
	vector< RMaterial > Materials;


	template<class LambdaType>
	void InitResources(const MMesh& Mesh, LambdaType&& PlatformFunc)
	{
		PlatformFunc(PositionVertexBuffer, UVVertexBuffer, NormalVertexBuffer, TangentVertexBuffer, IndexBuffer);

		assert((PositionVertexBuffer || UVVertexBuffer || NormalVertexBuffer || TangentVertexBuffer) && "Vertex buffers not initialized");
		PositionVertexBuffer->SetRawVertexBuffer(Mesh.RenderData.Positions);
		PositionVertexBuffer->AllocateResource();
		UVVertexBuffer->SetRawVertexBuffer(Mesh.RenderData.UV0);
		UVVertexBuffer->AllocateResource();
		IndexBuffer->SetIndexBuffer(Mesh.RenderData.Indices);
		IndexBuffer->AllocateResource();

		Sections = Mesh.Sections;
	}

	template<class SRVLambdaType>
	void InitMaterials(vector<MMaterial>& InMaterials, SRVLambdaType&& SRVFunc)
	{
		Materials.reserve(InMaterials.size());

		for (auto& Material : InMaterials)
		{
			int32 Index = Materials.size();
			Materials.emplace_back();

			RMaterial& MaterialProxy = Materials[Index];
			MaterialProxy.bVaild = Material.bValid;

			if (MaterialProxy.bVaild)
			{
				MaterialProxy.Textures.reserve(Material.Textures.size());

				for (auto& RawTexture : Material.Textures)
				{
					MaterialProxy.Textures.emplace_back();
					SRVFunc(MaterialProxy.Textures[MaterialProxy.Textures.size() - 1], RawTexture);
				}

				MaterialProxy.Colors.reserve(Material.Colors.size());

				for (const auto& Color : Material.Colors)
				{
					MaterialProxy.Colors.emplace_back(Color);
				}
			}
		}
	}

	~RMesh()
	{
		auto Delete = [](auto* Ptr)
		{
			if (Ptr)
			{
				delete Ptr;
			}
		};

		Delete(PositionVertexBuffer);
		Delete(UVVertexBuffer);
		Delete(NormalVertexBuffer);
		Delete(TangentVertexBuffer);
		Delete(IndexBuffer);
	}

	uint32 GetNumRegisteredTextures() const
	{
		uint32 NumTextures = 0;
		for (const auto& Material : Materials)
		{
			NumTextures += max(Material.Textures.size(), 1);
		}

		return NumTextures;
	}
};
