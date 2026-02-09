#pragma once
#include "../PlatformDefinitions.h"
#include "../RenderBackend/ResourceBuffer.h"
#include "../RenderBackend/DynamicBuffer.h"
#include "../RenderBackend/Texture.h"

#include "Texture.h"
#include <algorithm>

using MaterialId = uint32;
constexpr uint32 kMaterialConstantCount = 8;

struct Material
{
public:
	MaterialId Id = 0;
	uint32 bBaseColor = 0;
	uint32 bNormal = 0;
	uint32 bSpecular = 0;
	uint32 bShininess = 0;
	uint32 bAmbient = 0;
	uint32 bEmissive = 0;
	uint32 bReflection = 0;
	uint32 bMetallicRoughness = 0;
	uint32 BaseColorUVIndex = 0;
	uint32 NormalUVIndex = 0;
	uint32 MetallicRoughnessUVIndex = 0;
	uint32 ShininessUVIndex = 0;
	uint32 AmbientUVIndex = 0;
	uint32 EmissiveUVIndex = 0;

	// TO DO : REMOVE FROM HERE, CPU Texture Asset must be reside in somewhere other place that is proper to maintaining life time of texture cpu asset.
	vector<TextureAsset> Textures; // CPU asset textures (indexed by TextureType)
	vector< SharedPtr<TextureResource> > TexturesGPU; // GPU textures
	bool bValid = false;

	vector< float3 > Colors;
};

// Role-based naming aliases (unified material)
using MaterialAsset = Material;
using MaterialResource = Material;
using MaterialInstance = Material;

struct MRenderData
{
public:
	vector< float3 > Positions;
	vector< float4 > Colors;
	vector< vector< float2 > > UVChannels;
	vector< float3 > Normals;
	vector< float3 > Tangents;
	vector< float3 > Bitangents;
	vector< uint32 > Indices;

	vector<float2>& GetUVChannel(uint32 Index)
	{
		if (UVChannels.size() <= Index)
		{
			UVChannels.resize(Index + 1);
		}
		return UVChannels[Index];
	}

	const vector<float2>& GetUVChannel(uint32 Index) const
	{
		if (Index < UVChannels.size())
		{
			return UVChannels[Index];
		}

		static const vector<float2> Empty;
		return Empty;
	}

	uint32 GetNumUVChannels() const
	{
		return static_cast<uint32>(UVChannels.size());
	}
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
};

struct RMesh
{
public:
	uint32 NumVertices = 0;
	uint32 NumUVChannels = 0;
	bool bHasBounds = false;
	float3 BoundsMin{};
	float3 BoundsMax{};
	float3 BoundsCenter{};
	float3 BoundsExtent{};

	RVertexBuffer* PositionVertexBuffer;
	vector<RVertexBuffer*> UVVertexBuffers;
	RVertexBuffer* NormalVertexBuffer;
	RVertexBuffer* TangentVertexBuffer;
	RVertexBuffer* BitangentVertexBuffer;
	RIndexBuffer* IndexBuffer;

	vector< MSectionData > Sections;
	template<class LambdaType>
	void InitResources(const MMesh& Mesh, LambdaType&& PlatformFunc)
	{
		NumUVChannels = (std::max)(1u, Mesh.RenderData.GetNumUVChannels());
		PlatformFunc(PositionVertexBuffer, UVVertexBuffers, NormalVertexBuffer, TangentVertexBuffer, BitangentVertexBuffer, IndexBuffer, NumUVChannels);

		assert((PositionVertexBuffer || !UVVertexBuffers.empty() || NormalVertexBuffer || TangentVertexBuffer) && "Vertex buffers not initialized");
		NumVertices = Mesh.RenderData.Positions.size();

		if (!Mesh.RenderData.Positions.empty())
		{
			BoundsMin = Mesh.RenderData.Positions[0];
			BoundsMax = Mesh.RenderData.Positions[0];
			for (const auto& P : Mesh.RenderData.Positions)
			{
				BoundsMin.x = (std::min)(BoundsMin.x, P.x);
				BoundsMin.y = (std::min)(BoundsMin.y, P.y);
				BoundsMin.z = (std::min)(BoundsMin.z, P.z);
				BoundsMax.x = (std::max)(BoundsMax.x, P.x);
				BoundsMax.y = (std::max)(BoundsMax.y, P.y);
				BoundsMax.z = (std::max)(BoundsMax.z, P.z);
			}
			BoundsCenter = (BoundsMin + BoundsMax) * 0.5f;
			BoundsExtent = BoundsMax - BoundsMin;
			bHasBounds = true;
		}
		else
		{
			bHasBounds = false;
		}

		PositionVertexBuffer->SetRawVertexBuffer(Mesh.RenderData.Positions);
		PositionVertexBuffer->AllocateResource();

		vector<float2> DefaultUVs;
		DefaultUVs.resize(NumVertices, float2{ 0.0f, 0.0f });

		if (UVVertexBuffers.size() < NumUVChannels)
		{
			UVVertexBuffers.resize(NumUVChannels, nullptr);
		}

		for (uint32 ChannelIndex = 0; ChannelIndex < NumUVChannels; ++ChannelIndex)
		{
			const vector<float2>& Source = Mesh.RenderData.GetUVChannel(ChannelIndex);
			const vector<float2>& UVs = (Source.size() == NumVertices) ? Source : DefaultUVs;

			if (UVVertexBuffers[ChannelIndex])
			{
				UVVertexBuffers[ChannelIndex]->SetRawVertexBuffer(UVs);
				UVVertexBuffers[ChannelIndex]->AllocateResource();
			}
		}

		NormalVertexBuffer->SetRawVertexBuffer(Mesh.RenderData.Normals);
		NormalVertexBuffer->AllocateResource();

		TangentVertexBuffer->SetRawVertexBuffer(Mesh.RenderData.Tangents);
		TangentVertexBuffer->AllocateResource();

		BitangentVertexBuffer->SetRawVertexBuffer(Mesh.RenderData.Bitangents);
		BitangentVertexBuffer->AllocateResource();

		IndexBuffer->SetIndexBuffer(Mesh.RenderData.Indices);
		IndexBuffer->AllocateResource();

		Sections = Mesh.Sections;
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
		for (auto* UVBuffer : UVVertexBuffers)
		{
			Delete(UVBuffer);
		}
		Delete(NormalVertexBuffer);
		Delete(TangentVertexBuffer);
		Delete(BitangentVertexBuffer);
		Delete(IndexBuffer);
	}

	uint32 GetNumUVChannels() const
	{
		return NumUVChannels;
	}

};

// Role-based naming alias (render instance)
using MeshInstance = RMesh;
