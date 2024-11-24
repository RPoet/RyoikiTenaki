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


namespace floats
{
	float InnerProduct(const float3& RHS, const float3& LHS)
	{
		return RHS.x * LHS.x
			+ RHS.y * LHS.y
			+ RHS.z * LHS.z;
	}

	float InnerProduct(const float4& RHS, const float4& LHS)
	{
		return RHS.x * LHS.x
			+ RHS.y * LHS.y
			+ RHS.z * LHS.z
			+ RHS.w * LHS.w;
	}

	template<class T>
	float SqrDistance(const T& Vector)
	{
		return InnerProduct(Vector, Vector);
	}

	template<class T>
	float Distance(const T& Vector)
	{
		return std::sqrt(SqrDistance(Vector));
	}
};

#include <cmath>

float2 operator-(const float2& RHS, const float2& LHS)
{
	return float2
	{
		RHS.x - LHS.x,
		RHS.y - LHS.y,
	};
}

//float2 operator*(const float2& RHS, const float2& LHS)
//{
//	return float2
//	{
//		RHS.x * LHS.x,
//		RHS.y * LHS.y,
//	};
//}
//
//float2 operator/(const float2& RHS, const float2& LHS)
//{
//	return float2
//	{
//		RHS.x / LHS.x,
//		RHS.y / LHS.y,
//	};
//}

float3 operator-(const float3& RHS, const float3& LHS)
{
	return float3
	{
		RHS.x - LHS.x,
		RHS.y - LHS.y,
		RHS.z - LHS.z,
	};
}

float3 operator*(const float3& RHS, const float3& LHS)
{
	return float3
	{
		RHS.x * LHS.x,
		RHS.y * LHS.y,
		RHS.z * LHS.z,
	};
}

//float3 operator*(const float3& RHS, const float3& LHS)
//{
//	return float3
//	{
//		RHS.x * LHS.x,
//		RHS.y * LHS.y,
//		RHS.z * LHS.z,
//	};
//}
//
//float3 operator/(const float3& RHS, const float3& LHS)
//{
//	return float3
//	{
//		RHS.x / LHS.x,
//		RHS.y / LHS.y,
//		RHS.z / LHS.z,
//	};
//}

float4 operator-(const float4& RHS, const float4& LHS)
{
	return float4
	{
		RHS.x - LHS.x,
		RHS.y - LHS.y,
		RHS.z - LHS.z,
		RHS.w - LHS.w,
	};
}


//float4 operator/(const float4& RHS, const float4& LHS)
//{
//	return float4
//	{
//		RHS.x / LHS.x,
//		RHS.y / LHS.y,
//		RHS.z / LHS.z,
//		RHS.w / LHS.w,
//	};
//}
//
//float4 operator*(const float4& RHS, const float4& LHS)
//{
//	return float4
//	{
//		RHS.x * LHS.x,
//		RHS.y * LHS.y,
//		RHS.z * LHS.z,
//		RHS.w * LHS.w,
//	};
//}

float3 Normalize(const float3& Vector)
{
	const float Distance = floats::Distance(Vector);
	if (Distance != 0)
	{
		return float3
		{
		  Vector.x / Distance
		, Vector.y / Distance 
		, Vector.z / Distance
		};
	}

	return float3(0, 0, 0);
}

float4 Normalize(const float4& Vector)
{
	const float Distance = floats::Distance(Vector);
	if (Distance != 0)
	{
		return float4
		{
		  Vector.x / Distance
		, Vector.y / Distance
		, Vector.z / Distance
		, Vector.w / Distance
		};
	}

	return float4(0, 0, 0, 0);
}

float3 Cross(const float3& RHS, const float3& LHS)
{
	return float3
	{
		RHS.y * LHS.z - RHS.z * LHS.y,
		RHS.z * LHS.x - RHS.x * LHS.z,
		RHS.x * LHS.y - RHS.y * LHS.x,
	};
}

bool IsNormalized(const float4& Vector)
{
	const float Distance = floats::Distance(Vector);
	return std::abs(1 - Distance) < 0.0001f;
}


bool IsNormalized(const float3& Vector)
{
	const float Distance = floats::Distance(Vector);
	return std::abs(1 - Distance) < 0.0001f;
}

struct Vertex
{
	float3 position;
	float3 normal;
	float3 tangent;
	float3 bitangent;
	float2 texCoord;

	bool operator==(const Vertex& other) const {
		return position == other.position
			&& normal == other.normal
			&& tangent == other.tangent
			&& bitangent == other.bitangent
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
        return ((((((hash<float3>()(vertex.position) ^ 
                    (hash<float3>()(vertex.normal) << 1)) >> 1) ^
                    (hash<float2>()(vertex.texCoord) << 1)) >> 1) ^
                    (hash<float3>()(vertex.tangent) << 1)) >> 1) ^
                    (hash<float3>()(vertex.bitangent) << 1);
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

		for (int32 j = 0; j < Mesh.indices.size(); j += 3)
		{
			Vertex vertex[3]{};
			
			for (int32 iTri = 0; iTri < 3; ++iTri)
			{
				tinyobj::index_t i = Mesh.indices[j + iTri];

				float3 Position = {
					attributes.vertices[i.vertex_index * 3],
					attributes.vertices[i.vertex_index * 3 + 1],
					attributes.vertices[i.vertex_index * 3 + 2]
				};
				float3 Normal = {
					attributes.normals[i.normal_index * 3],
					attributes.normals[i.normal_index * 3 + 1],
					attributes.normals[i.normal_index * 3 + 2]
				};

				float3 Tangent{};

				float3 BiTangent{};

				float2 TexCoord = {
					attributes.texcoords[i.texcoord_index * 2],
					attributes.texcoords[i.texcoord_index * 2 + 1],
				};

				vertex[iTri] = { Position, Normal, Tangent, BiTangent, TexCoord };
			}


			float3 TriTangent{};
			float3 BiTangent{};
			{
				// Calc Tangent
				float3 E0 = vertex[1].position - vertex[0].position;
				float3 E1 = vertex[2].position - vertex[0].position;

				float2 dUV0 = vertex[1].texCoord - vertex[0].texCoord;
				float2 dUV1 = vertex[2].texCoord - vertex[0].texCoord;

				float Det = dUV1.x * dUV0.y - dUV0.x * dUV1.y;
				if (Det == 0)
				{
					TriTangent = { 1.0f, 0.0f, 0.0f };
					BiTangent = { 0.0f, 1.0f, 0.0f };
				}
				else
				{
					// Determinant dUV Matrix.
					float f = 1.0f / Det;

					// Simple matrix calculation.
					TriTangent.x = f * (dUV1.y * E0.x - dUV0.y * E1.x);
					TriTangent.y = f * (dUV1.y * E0.y - dUV0.y * E1.y);
					TriTangent.z = f * (dUV1.y * E0.z - dUV0.y * E1.z);

					BiTangent.x = f * (-dUV1.x * E0.x + dUV0.x * E1.x);
					BiTangent.y = f * (-dUV1.x * E0.y + dUV0.x * E1.y);
					BiTangent.z = f * (-dUV1.x * E0.z + dUV0.x * E1.z);

					TriTangent = Normalize(TriTangent);
					BiTangent = Normalize(BiTangent);
				}
			}


			auto GramSchmidt = [](float3& Tangent, float3& Bitangent, const float3& Normal)
			{
				float Dot = floats::InnerProduct(Tangent, Normal);
				Tangent = Normalize(Tangent - Normal * float3(Dot, Dot, Dot));
				Bitangent = Normalize(Cross(Normal, Tangent));
			};

			for (int32 iTri = 0; iTri < 3; ++iTri)
			{
				vertex[iTri].tangent = TriTangent;
				vertex[iTri].bitangent = BiTangent;
				GramSchmidt(vertex[iTri].tangent, vertex[iTri].bitangent, vertex[iTri].normal);

				if (UniqueVertices.count(vertex[iTri]) == 0)
				{
					UniqueVertices[vertex[iTri]] = static_cast<uint32_t>(Out.RenderData.Positions.size());

					Out.RenderData.Positions.push_back(vertex[iTri].position);
					Out.RenderData.UV0.push_back(vertex[iTri].texCoord);
					Out.RenderData.Normals.push_back(vertex[iTri].normal);
					Out.RenderData.Tangents.push_back(vertex[iTri].tangent);
					Out.RenderData.Bitangents.push_back(vertex[iTri].bitangent);
				}
			}

			Out.RenderData.Indices.push_back(UniqueVertices[vertex[0]]);
			Out.RenderData.Indices.push_back(UniqueVertices[vertex[1]]);
			Out.RenderData.Indices.push_back(UniqueVertices[vertex[2]]);
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
