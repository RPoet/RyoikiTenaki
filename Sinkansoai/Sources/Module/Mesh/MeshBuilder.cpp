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
	std::vector<tinyobj::shape_t> Shapes; // section.
	std::vector<tinyobj::material_t> Materials;
	std::string warnings;
	std::string errors;

	tinyobj::LoadObj(&attributes, &Shapes, &Materials, &warnings, &errors, CharPath.c_str(), Mtl.c_str());

	unordered_map<Vertex, uint32_t> UniqueVertices = {};

	std::array<uint32, 3> FaceIndices;

	//TextureBuilder;
	std::set<uint32> MaterialSet;
	for (auto& Shape : Shapes)
	{
		auto& Mesh = Shape.mesh;
		int32 NumAddedIndices = 0;
		
		int32 IndexStart = Out.RenderData.Indices.size();

		MaterialSet.clear();

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
				Out.RenderData.Normals.push_back(vertex.normal);
			}


			//MaterialSet.insert();

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
		int32 MaterialId = Mesh.material_ids[0]; // Assume material id per section would be same.
		MSectionData SectionData{ IndexStart, IndexEnd,  MaterialId };
		Out.Sections.push_back(SectionData);
	}


	{
		auto&& TexturePathBase = Path;

		for (auto& Material : Materials)
		{
			uint32 Index = Out.Materials.size();
			Out.Materials.emplace_back();
			Out.Materials[Index].bValid = false;
			if (Material.diffuse_texname.length() > 0)
			{
				Out.Materials[Index].bValid = true;

				auto&& DiffuseTexture = MTextureBuilder::Get().LoadTexture(TexturePathBase, String(Material.diffuse_texname.begin(), Material.diffuse_texname.end()));
				Out.Materials[Index].Textures.push_back(DiffuseTexture);
				Out.Materials[Index].Colors.emplace_back(Material.diffuse[0], Material.diffuse[1], Material.diffuse[2]);

				Out.Materials[Index].bBaseColor = true;
			}

			if (Material.displacement_texname.length() > 0)
			{
				Out.Materials[Index].bValid = true;

				auto&& Normal = MTextureBuilder::Get().LoadTexture(TexturePathBase, String(Material.displacement_texname.begin(), Material.displacement_texname.end()));
				Out.Materials[Index].Textures.push_back(Normal);
				Out.Materials[Index].Colors.emplace_back(1,1,1);


				Out.Materials[Index].bNormal = true;
			}
		}
	}
	
	return Out;
}
