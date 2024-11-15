#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>

#include "MeshBuilder.h"
#include "TinyObjLoader.h"
#include "../../Engine.h"

#include "../Texture/TextureBuilder.h"

IMPLEMENT_MODULE(MMeshBuilder)

bool operator==(const float3& A, const float3& B)
{
	return A.x == B.x && A.y == B.y && A.z == B.z;
}


bool operator==(const float2& A, const float2& B)
{
	return A.x == B.x && A.y == B.y;
}


struct Vertex
{
	float3 position;
	float3 normal;
	float2 texCoord;

	bool operator==(const Vertex& other) const {
		return position == other.position
			&& normal == other.normal
			&& texCoord == other.texCoord;
	}
};


namespace std {
	template<> struct hash<float3>
	{
		size_t operator()(const float3& v) const {
			return ((hash<float>()(v.x) ^ (hash<float>()(v.y) << 1)) >> 1) ^ (hash<float>()(v.z) << 1);
		}
	};

	template<> struct hash<float2>
	{
		size_t operator()(const float2& v) const {
			return (std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1));
		}
	};

	template<> struct hash<Vertex> {
		size_t operator()(const Vertex& vertex) const {
			return ((hash<float3>()(vertex.position) ^
				(hash<float3>()(vertex.normal) << 1)) >> 1) ^
				(hash<float2>()(vertex.texCoord) << 1);
		}
	};
}

void MMeshBuilder::Init()
{
	Super::Init();
	
	cout << "Mesh Builder Init" << endl;

}


void MMeshBuilder::Teardown()
{
	Super::Teardown();
}

MMesh MMeshBuilder::LoadMesh(const String& Path, const String& ModelName)
{
	MMesh Out{};

	auto ConnectedString = std::wstring(Path + ModelName);

	std::string Mtl(Path.begin(), Path.end());
	std::string CharPath(ConnectedString.begin(), ConnectedString.end());

	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes; // section.
	std::vector<tinyobj::material_t> materials;
	std::string warnings;
	std::string errors;

	tinyobj::LoadObj(&attributes, &shapes, &materials, &warnings, &errors, CharPath.c_str(), Mtl.c_str());

	unordered_map<Vertex, uint32_t> UniqueVertices = {};

	std::array<uint32, 3> FaceIndices;

	//TextureBuilder;

	for (auto& Shape : shapes)
	{
		auto& Mesh = Shape.mesh;
		int32 NumAddedIndices = 0;
		
		int32 IndexStart = Out.RenderData.Indices.size();

		for (int j = 0; j < Mesh.indices.size(); j++)
		{
			tinyobj::index_t i = Mesh.indices[j];

			float3 position = {
				attributes.vertices[i.vertex_index * 3],
				attributes.vertices[i.vertex_index * 3 + 1],
				attributes.vertices[i.vertex_index * 3 + 2]
			};
			float3 normal = {
				attributes.normals[i.normal_index * 3],
				attributes.normals[i.normal_index * 3 + 1],
				attributes.normals[i.normal_index * 3 + 2]
			};
			float2 texCoord = {
				attributes.texcoords[i.texcoord_index * 2],
				attributes.texcoords[i.texcoord_index * 2 + 1],
			};

			Vertex vertex = { position, normal, texCoord };

			if (UniqueVertices.count(vertex) == 0) {
				UniqueVertices[vertex] = static_cast<uint32_t>(Out.RenderData.Positions.size());

				Out.RenderData.Positions.push_back(vertex.position);
				Out.RenderData.UV0.push_back(vertex.texCoord);
			}

			FaceIndices[NumAddedIndices++] = UniqueVertices[vertex];

			if (NumAddedIndices == 3)
			{
				NumAddedIndices = 0;
				Out.RenderData.Indices.push_back(FaceIndices[0]);
				Out.RenderData.Indices.push_back(FaceIndices[1]);
				Out.RenderData.Indices.push_back(FaceIndices[2]);
			}
		}

		int32 IndexEnd = Out.RenderData.Indices.size();

		MSectionData SectionData{ IndexStart, IndexEnd };
		Out.Sections.push_back(SectionData);
	}

	return Out;
}
