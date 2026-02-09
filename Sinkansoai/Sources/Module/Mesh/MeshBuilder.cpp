#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <algorithm>
#include <cassert>

#include "MeshBuilder.h"
#include "TinyObjLoader.h"
#include "ThirdParty/OpenFBX/ofbx.h"
#include "../../Engine.h"

#include "../Texture/TextureBuilder.h"

IMPLEMENT_MODULE(MMeshBuilder)

namespace floats
{
	float InnerProduct(const float3& RHS, const float3& LHS)
	{
        // Use dot product from SIMDMath
		return float3::Dot(RHS, LHS);
	}

	float InnerProduct(const float4& RHS, const float4& LHS)
	{
		// Manual dot for float4 if not in SIMDMath (Vector4 didn't have Dot in my generic impl? let's check. Vector3 had Dot. Vector4... checking SIMDMath.h content from step 35/40... Vector4 had no Dot.
        // So I'll implement it manually here or use SIMD access)
        return RHS.x * LHS.x + RHS.y * LHS.y + RHS.z * LHS.z + RHS.w * LHS.w;
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

float3 Normalize(const float3& Vector)
{
    return float3::Normalize(Vector);
}

// float4 Normalize... Vector4 didn't have Normalize.
// Keep generic Normalize for float4 if needed, or implement using SIMD logic?
// Keeping existing scalar implementation for float4 is safe for now to reduce changes.
float4 Normalize(const float4& Vector)
{
	const float Distance = floats::Distance(Vector);
	if (Distance != 0)
	{
		return Vector / Distance;
	}
	return float4(0, 0, 0, 0);
}

float3 Cross(const float3& RHS, const float3& LHS)
{
    return float3::Cross(RHS, LHS);
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
	float2 texCoord1;

	bool operator==(const Vertex& other) const {
		return position == other.position
			&& normal == other.normal
			&& tangent == other.tangent
			&& bitangent == other.bitangent
			&& texCoord == other.texCoord
			&& texCoord1 == other.texCoord1;
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
                    (hash<float3>()(vertex.bitangent) << 1) ^
					(hash<float2>()(vertex.texCoord1) << 2);
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

vector<MMesh> MMeshBuilder::LoadMesh(const String& Path, const String& ModelName, vector<Material>& OutMaterials)
{
	vector<MMesh> OutMeshes;
	OutMaterials.clear();

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

				vertex[iTri] = { Position, Normal, Tangent, BiTangent, TexCoord, TexCoord };
			}


			float3 TriTangent{};
			float3 BiTangent{};
			{
				// Calc Tangent
				float3 E0 = vertex[1].position - vertex[0].position;
				float3 E1 = vertex[2].position - vertex[0].position;

				float2 dUV0 = vertex[1].texCoord - vertex[0].texCoord;
				float2 dUV1 = vertex[2].texCoord - vertex[0].texCoord;

				float U0 = dUV0.x;
				float V0 = dUV0.y;

				float U1 = dUV1.x;
				float V1 = dUV1.y;

				float Det = U0 * V1 - V0 * U1;
				if (Det == 0)
				{
					TriTangent = { -1.0f, 0.0f, 0.0f };
					BiTangent = { 0.0f, -1.0f, 0.0f };
				}
				else
				{
					// Determinant dUV Matrix.
					float f = 1.0f / Det;

					// Simple matrix calculation.
					TriTangent.x = f * ( ( V1 * E0.x ) + ( -V0 * E1.x ) );
					TriTangent.y = f * ( ( V1 * E0.y ) + ( -V0 * E1.y ) );
					TriTangent.z = f * ( ( V1 * E0.z ) + ( -V0 * E1.z ) );

					BiTangent.x = f * (( -U1 * E0.x ) + ( U0 * E1.x ));
					BiTangent.y = f * (( -U1 * E0.y ) + ( U0 * E1.y ));
					BiTangent.z = f * (( -U1 * E0.z ) + ( U0 * E1.z ));

					TriTangent = Normalize(TriTangent);
					BiTangent = Normalize(BiTangent);
				}
			}

			for (int32 iTri = 0; iTri < 3; ++iTri)
			{
				vertex[iTri].tangent = TriTangent;
				vertex[iTri].bitangent = BiTangent;
			}


			auto GramSchmidt = [](float3& Tangent, float3& Bitangent, const float3& Normal)
			{
				float Dot = floats::InnerProduct(Tangent, Normal);
				Tangent = Normalize(Tangent - Normal * Dot);
				Bitangent = Normalize(Cross(Normal, Tangent));
			};

			for (int32 iTri = 0; iTri < 3; ++iTri)
			{
				//GramSchmidt(vertex[iTri].tangent, vertex[iTri].bitangent, vertex[iTri].normal);

				if (UniqueVertices.count(vertex[iTri]) == 0)
				{
					UniqueVertices[vertex[iTri]] = static_cast<uint32_t>(Out.RenderData.Positions.size());

					Out.RenderData.Positions.push_back(vertex[iTri].position);
					Out.RenderData.GetUVChannel(0).push_back(vertex[iTri].texCoord);
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
		int32 MaterialId = 0;
		if (!Mesh.material_ids.empty())
		{
			MaterialId = Mesh.material_ids[0]; // Assume material id per section would be same.
		}
		MSectionData SectionData{ IndexStart, IndexEnd, static_cast<uint32>(MaterialId) };
		Out.Sections.push_back(SectionData);
	}

	{
		auto&& TexturePathBase = Path;

		OutMaterials.reserve(Materials.size());
		for (auto& MaterialData : Materials)
		{
			Material OutMat{};
			OutMat.bValid = false;
			OutMat.Textures.resize(TextureType::COUNT);

			auto LoadTextureSlot = [&](TextureType Slot, const std::string& Name)
			{
				auto&& Texture = MTextureBuilder::Get().LoadTexture(TexturePathBase, String(Name.begin(), Name.end()));
				Texture.bSRGB = IsSRGBTextureType(Slot);
				OutMat.Textures[Slot] = std::move(Texture);
				OutMat.bValid = true;
			};

			if (!MaterialData.diffuse_texname.empty())
			{
				LoadTextureSlot(DIFFUSE, MaterialData.diffuse_texname);
				OutMat.Colors.emplace_back(MaterialData.diffuse[0], MaterialData.diffuse[1], MaterialData.diffuse[2]);
				OutMat.bBaseColor = true;
			}

			std::string NormalTexName;
			if (!MaterialData.normal_texname.empty())
			{
				NormalTexName = MaterialData.normal_texname;
			}
			else if (!MaterialData.bump_texname.empty())
			{
				NormalTexName = MaterialData.bump_texname;
			}
			else if (!MaterialData.displacement_texname.empty())
			{
				NormalTexName = MaterialData.displacement_texname;
			}

			if (!NormalTexName.empty())
			{
				LoadTextureSlot(NORMAL, NormalTexName);
				OutMat.Colors.emplace_back(1, 1, 1);
				OutMat.bNormal = true;
			}

			if (!MaterialData.metallic_texname.empty())
			{
				LoadTextureSlot(SPECULAR, MaterialData.metallic_texname);
				OutMat.bSpecular = true;
			}
			else if (!MaterialData.specular_texname.empty())
			{
				LoadTextureSlot(SPECULAR, MaterialData.specular_texname);
				OutMat.bSpecular = true;
			}

			if (!MaterialData.roughness_texname.empty())
			{
				auto Roughness = MTextureBuilder::Get().LoadTexture(TexturePathBase, String(MaterialData.roughness_texname.begin(), MaterialData.roughness_texname.end()));
				Roughness.bSRGB = IsSRGBTextureType(SHININESS);
				for (size_t i = 0; i + 3 < Roughness.Pixels.size(); i += 4)
				{
					Roughness.Pixels[i + 0] = static_cast<uint8>(255 - Roughness.Pixels[i + 0]);
					Roughness.Pixels[i + 1] = static_cast<uint8>(255 - Roughness.Pixels[i + 1]);
					Roughness.Pixels[i + 2] = static_cast<uint8>(255 - Roughness.Pixels[i + 2]);
				}
				OutMat.Textures[SHININESS] = std::move(Roughness);
				OutMat.bShininess = true;
				OutMat.bValid = true;
			}
			else if (!MaterialData.specular_highlight_texname.empty())
			{
				LoadTextureSlot(SHININESS, MaterialData.specular_highlight_texname);
				OutMat.bShininess = true;
			}

			if (!MaterialData.ambient_texname.empty())
			{
				LoadTextureSlot(AMBIENT, MaterialData.ambient_texname);
				OutMat.bAmbient = true;
			}

			if (!MaterialData.emissive_texname.empty())
			{
				LoadTextureSlot(EMISSIVE, MaterialData.emissive_texname);
				OutMat.bEmissive = true;
			}

			if (!MaterialData.reflection_texname.empty())
			{
				LoadTextureSlot(REFLECTION, MaterialData.reflection_texname);
				OutMat.bReflection = true;
			}

			OutMaterials.emplace_back(std::move(OutMat));
		}
	}

	if (OutMaterials.empty())
	{
		OutMaterials.emplace_back();
	}

	const uint32 MaxMaterialIndex = static_cast<uint32>((std::max)(static_cast<size_t>(1), OutMaterials.size()) - 1);
	for (auto& Section : Out.Sections)
	{
		if (Section.MaterialId > MaxMaterialIndex)
		{
			Section.MaterialId = 0;
		}
	}

	OutMeshes.emplace_back(std::move(Out));
	return OutMeshes;
}

void LogGlobalSettingsOnce(const ofbx::GlobalSettings* Settings)
{
	static bool bLogged = false;
	if (bLogged || !Settings)
	{
		return;
	}
	bLogged = true;
	cout << "FBX GlobalSettings: UpAxis=" << Settings->UpAxis
		<< " UpSign=" << Settings->UpAxisSign
		<< " FrontAxis=" << Settings->FrontAxis
		<< " FrontSign=" << Settings->FrontAxisSign
		<< " CoordAxis=" << Settings->CoordAxis
		<< " CoordSign=" << Settings->CoordAxisSign
		<< " UnitScale=" << Settings->UnitScaleFactor
		<< endl;
};


vector<MMesh> MMeshBuilder::LoadMeshFBX(const String& Path, const String& ModelName, vector<Material>& OutMaterials)
{
	vector<MMesh> OutMeshes;
	OutMaterials.clear();

	const String Combined = Path + ModelName;
	const std::string FilePath(Combined.begin(), Combined.end());

	std::ifstream File(FilePath, std::ios::binary);
	if (!File)
	{
		cout << "FBX: Failed to open file: " << FilePath << endl;
		return OutMeshes;
	}

	File.seekg(0, std::ios::end);
	const size_t Size = static_cast<size_t>(File.tellg());
	File.seekg(0, std::ios::beg);

	vector<ofbx::u8> Data(Size);
	if (!File.read(reinterpret_cast<char*>(Data.data()), Size))
	{
		cout << "FBX: Failed to read file: " << FilePath << endl;
		return OutMeshes;
	}

	ofbx::IScene* Scene = ofbx::load(Data.data(), Data.size(), static_cast<ofbx::u16>(ofbx::LoadFlags::KEEP_MATERIAL_MAP));
	if (!Scene)
	{
		cout << "FBX: " << ofbx::getError() << endl;
		return OutMeshes;
	}
	LogGlobalSettingsOnce(Scene->getGlobalSettings());

	unordered_map<const ofbx::Material*, uint32> MaterialLookup = {};

	auto ToWideString = [](const ofbx::DataView& View) -> String
	{
		if (!View.begin || View.begin == View.end)
		{
			return String();
		}

		std::string Temp(reinterpret_cast<const char*>(View.begin), reinterpret_cast<const char*>(View.end));
		return String(Temp.begin(), Temp.end());
	};

	auto IsAbsolutePath = [](const String& InPath) -> bool
	{
		if (InPath.size() >= 2 && InPath[1] == L':')
		{
			return true;
		}
		if (!InPath.empty() && (InPath[0] == L'/' || InPath[0] == L'\\'))
		{
			return true;
		}
		return false;
	};

	auto ToRowMajor = [](const ofbx::DMatrix& M) -> SIMDMath::Matrix4x4
	{
		SIMDMath::Matrix4x4 R;
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				// ofbx uses column-major; convert to row-vector convention by transposing.
				R.m[row][col] = static_cast<float>(M.m[col + row * 4]);
			}
		}
		return R;
	};

	auto MulDMatrix = [](const ofbx::DMatrix& A, const ofbx::DMatrix& B) -> ofbx::DMatrix
	{
		ofbx::DMatrix R{};
		for (int col = 0; col < 4; ++col)
		{
			for (int row = 0; row < 4; ++row)
			{
				double sum = 0.0;
				for (int k = 0; k < 4; ++k)
				{
					sum += A.m[row + k * 4] * B.m[k + col * 4];
				}
				R.m[row + col * 4] = sum;
			}
		}
		return R;
	};

	auto BuildAxisConversion = [](const ofbx::GlobalSettings* Settings, bool& OutFlipWinding) -> SIMDMath::Matrix4x4
	{
		using SIMDMath::Matrix4x4;
		OutFlipWinding = false;

		if (!Settings)
		{
			return Matrix4x4::Identity();
		}

		auto AxisIndex = [](int Axis) -> int
		{
			if (Axis == 0 || Axis == 1 || Axis == 2)
			{
				return Axis;
			}
			switch (Axis)
			{
			case ofbx::UpVector_AxisX: return 0;
			case ofbx::UpVector_AxisY: return 1;
			case ofbx::UpVector_AxisZ: return 2;
			default: return 1;
			}
		};

		int RightAxis = AxisIndex(static_cast<int>(Settings->CoordAxis));
		const int UpAxis = AxisIndex(static_cast<int>(Settings->UpAxis));
		int FwdAxis = AxisIndex(static_cast<int>(Settings->FrontAxis));

		auto RemainingAxis = [](int A, int B) -> int
		{
			for (int i = 0; i < 3; ++i)
			{
				if (i != A && i != B)
				{
					return i;
				}
			}
			return 2;
		};

		if (RightAxis == UpAxis)
		{
			RightAxis = RemainingAxis(UpAxis, FwdAxis);
		}

		if (FwdAxis == UpAxis || FwdAxis == RightAxis)
		{
			FwdAxis = RemainingAxis(UpAxis, RightAxis);
		}

		const float RightSign = (Settings->CoordAxisSign >= 0) ? 1.0f : -1.0f;
		const float UpSign = (Settings->UpAxisSign >= 0) ? 1.0f : -1.0f;
		const float FwdSign = (Settings->FrontAxisSign >= 0) ? 1.0f : -1.0f;

		Matrix4x4 M = Matrix4x4::Identity();
		// Row j = FBX axis j expressed in engine axes (row-vector convention).
		M.m[RightAxis][0] = RightSign;
		M.m[UpAxis][1] = UpSign;
		M.m[FwdAxis][2] = FwdSign;

		const float Det =
			M.m[0][0] * (M.m[1][1] * M.m[2][2] - M.m[1][2] * M.m[2][1]) -
			M.m[0][1] * (M.m[1][0] * M.m[2][2] - M.m[1][2] * M.m[2][0]) +
			M.m[0][2] * (M.m[1][0] * M.m[2][1] - M.m[1][1] * M.m[2][0]);
		OutFlipWinding = (Det < 0.0f);

		return M;
	};

	auto AppendAxisBox = [](MMesh& Target, const float3& Min, const float3& Max, uint32 MaterialId)
	{
		auto AddQuad = [&](const float3& A, const float3& B, const float3& C, const float3& D, const float3& N, const float3& T, const float3& Bn)
		{
			const uint32 Start = static_cast<uint32>(Target.RenderData.Positions.size());
			Target.RenderData.Positions.push_back(A);
			Target.RenderData.Positions.push_back(B);
			Target.RenderData.Positions.push_back(C);
			Target.RenderData.Positions.push_back(D);

			Target.RenderData.Normals.push_back(N);
			Target.RenderData.Normals.push_back(N);
			Target.RenderData.Normals.push_back(N);
			Target.RenderData.Normals.push_back(N);

			Target.RenderData.Tangents.push_back(T);
			Target.RenderData.Tangents.push_back(T);
			Target.RenderData.Tangents.push_back(T);
			Target.RenderData.Tangents.push_back(T);

			Target.RenderData.Bitangents.push_back(Bn);
			Target.RenderData.Bitangents.push_back(Bn);
			Target.RenderData.Bitangents.push_back(Bn);
			Target.RenderData.Bitangents.push_back(Bn);

			Target.RenderData.GetUVChannel(0).push_back(float2{ 0.0f, 0.0f });
			Target.RenderData.GetUVChannel(0).push_back(float2{ 1.0f, 0.0f });
			Target.RenderData.GetUVChannel(0).push_back(float2{ 1.0f, 1.0f });
			Target.RenderData.GetUVChannel(0).push_back(float2{ 0.0f, 1.0f });

			Target.RenderData.Indices.push_back(Start + 0);
			Target.RenderData.Indices.push_back(Start + 1);
			Target.RenderData.Indices.push_back(Start + 2);
			Target.RenderData.Indices.push_back(Start + 0);
			Target.RenderData.Indices.push_back(Start + 2);
			Target.RenderData.Indices.push_back(Start + 3);
		};

		const float3 A = { Min.x, Min.y, Min.z };
		const float3 B = { Max.x, Min.y, Min.z };
		const float3 C = { Max.x, Max.y, Min.z };
		const float3 D = { Min.x, Max.y, Min.z };
		const float3 E = { Min.x, Min.y, Max.z };
		const float3 F = { Max.x, Min.y, Max.z };
		const float3 G = { Max.x, Max.y, Max.z };
		const float3 H = { Min.x, Max.y, Max.z };

		const uint32 StartIndex = static_cast<uint32>(Target.RenderData.Indices.size());
		AddQuad(A, B, C, D, float3{ 0, 0, -1 }, float3{ 1, 0, 0 }, float3{ 0, 1, 0 });
		AddQuad(F, E, H, G, float3{ 0, 0, 1 }, float3{ -1, 0, 0 }, float3{ 0, 1, 0 });
		AddQuad(E, A, D, H, float3{ -1, 0, 0 }, float3{ 0, 0, -1 }, float3{ 0, 1, 0 });
		AddQuad(B, F, G, C, float3{ 1, 0, 0 }, float3{ 0, 0, 1 }, float3{ 0, 1, 0 });
		AddQuad(D, C, G, H, float3{ 0, 1, 0 }, float3{ 1, 0, 0 }, float3{ 0, 0, 1 });
		AddQuad(E, F, B, A, float3{ 0, -1, 0 }, float3{ 1, 0, 0 }, float3{ 0, 0, -1 });

		const uint32 EndIndex = static_cast<uint32>(Target.RenderData.Indices.size());
		Target.Sections.push_back(MSectionData{ StartIndex, EndIndex, MaterialId });
	};

	auto EnsureAxisMaterials = [](vector<Material>& Materials) -> std::array<uint32, 3>
	{
		auto MakeColorTex = [](const wchar_t* Name, uint8 B, uint8 G, uint8 R, uint8 A) -> MTexture
		{
			MTexture T{};
			T.Name = Name;
			T.Width = 1;
			T.Height = 1;
			T.BitsPerPixel = 32;
			T.Size = 0;
			T.Pixels = { B, G, R, A };
			return T;
		};

		auto MakeNormalTex = []() -> MTexture
		{
			MTexture T{};
			T.Name = TEXT("AxisNormal");
			T.Width = 1;
			T.Height = 1;
			T.BitsPerPixel = 32;
			T.Size = 0;
			T.Pixels = { 255, 128, 128, 255 }; // BGRA for (0.5,0.5,1)
			return T;
		};

		const uint32 Base = static_cast<uint32>(Materials.size());
		Materials.resize(Base + 3);

		const MTexture NormalTex = MakeNormalTex();

		Materials[Base + 0].bBaseColor = 1;
		Materials[Base + 0].bNormal = 1;
		Materials[Base + 0].bValid = true;
		Materials[Base + 0].Textures.resize(TextureType::COUNT);
		Materials[Base + 0].Textures[DIFFUSE] = MakeColorTex(TEXT("AxisX"), 0, 0, 255, 255);
		Materials[Base + 0].Textures[DIFFUSE].bSRGB = IsSRGBTextureType(DIFFUSE);
		Materials[Base + 0].Textures[NORMAL] = NormalTex;
		Materials[Base + 0].Textures[NORMAL].bSRGB = IsSRGBTextureType(NORMAL);

		Materials[Base + 1].bBaseColor = 1;
		Materials[Base + 1].bNormal = 1;
		Materials[Base + 1].bValid = true;
		Materials[Base + 1].Textures.resize(TextureType::COUNT);
		Materials[Base + 1].Textures[DIFFUSE] = MakeColorTex(TEXT("AxisY"), 0, 255, 0, 255);
		Materials[Base + 1].Textures[DIFFUSE].bSRGB = IsSRGBTextureType(DIFFUSE);
		Materials[Base + 1].Textures[NORMAL] = NormalTex;
		Materials[Base + 1].Textures[NORMAL].bSRGB = IsSRGBTextureType(NORMAL);

		Materials[Base + 2].bBaseColor = 1;
		Materials[Base + 2].bNormal = 1;
		Materials[Base + 2].bValid = true;
		Materials[Base + 2].Textures.resize(TextureType::COUNT);
		Materials[Base + 2].Textures[DIFFUSE] = MakeColorTex(TEXT("AxisZ"), 255, 0, 0, 255);
		Materials[Base + 2].Textures[DIFFUSE].bSRGB = IsSRGBTextureType(DIFFUSE);
		Materials[Base + 2].Textures[NORMAL] = NormalTex;
		Materials[Base + 2].Textures[NORMAL].bSRGB = IsSRGBTextureType(NORMAL);

		return { Base + 0, Base + 1, Base + 2 };
	};

	auto GetMaterialIndex = [&](const ofbx::Material* Material) -> uint32
	{
		if (!Material)
		{
			return 0;
		}

		auto Found = MaterialLookup.find(Material);
		if (Found != MaterialLookup.end())
		{
			return Found->second;
		}

		const uint32 NewIndex = static_cast<uint32>(OutMaterials.size());
		OutMaterials.emplace_back();

		auto& OutMat = OutMaterials.back();
		OutMat.bBaseColor = 0;
		OutMat.bNormal = 0;
		OutMat.bSpecular = 0;
		OutMat.bShininess = 0;
		OutMat.bAmbient = 0;
		OutMat.bEmissive = 0;
		OutMat.bReflection = 0;
		OutMat.bMetallicRoughness = 0;
		OutMat.BaseColorUVIndex = 0;
		OutMat.NormalUVIndex = 0;
		OutMat.MetallicRoughnessUVIndex = 0;
		OutMat.ShininessUVIndex = 0;
		OutMat.AmbientUVIndex = 0;
		OutMat.EmissiveUVIndex = 0;
		OutMat.bValid = false;
		OutMat.Textures.resize(TextureType::COUNT);

		{
			const ofbx::Color Diffuse = Material->getDiffuseColor();
			const float DiffuseFactor = static_cast<float>(Material->getDiffuseFactor());
			float3 Color = float3(
				static_cast<float>(Diffuse.r) * DiffuseFactor,
				static_cast<float>(Diffuse.g) * DiffuseFactor,
				static_cast<float>(Diffuse.b) * DiffuseFactor);

			OutMat.Colors.emplace_back(Color);
			OutMat.bValid = true;
		}

		auto NarrowString = [](const String& InText)
		{
			return std::string(InText.begin(), InText.end());
		};

		auto ToTextureType = [](ofbx::Texture::TextureType Type) -> TextureType
		{
			switch (Type)
			{
			case ofbx::Texture::DIFFUSE: return DIFFUSE;
			case ofbx::Texture::NORMAL: return NORMAL;
			case ofbx::Texture::SPECULAR: return SPECULAR;
			case ofbx::Texture::SHININESS: return SHININESS;
			case ofbx::Texture::AMBIENT: return AMBIENT;
			case ofbx::Texture::EMISSIVE: return EMISSIVE;
			case ofbx::Texture::REFLECTION: return REFLECTION;
			default: return DIFFUSE;
			}
		};

		auto LoadTextureIfAny = [&](ofbx::Texture::TextureType Type, const char* Label, bool& bEmbedded, bool& bLoaded)
		{
			bEmbedded = false;
			bLoaded = false;

			const ofbx::Texture* Texture = Material->getTexture(Type);
			if (!Texture)
			{
				return String();
			}

			const ofbx::DataView Embedded = Texture->getEmbeddedData();
			if (Embedded.begin && Embedded.end && Embedded.end > Embedded.begin)
			{
				bEmbedded = true;
			}

			String Relative = ToWideString(Texture->getRelativeFileName());
			String Absolute = ToWideString(Texture->getFileName());
			String Chosen = !Relative.empty() ? Relative : Absolute;
			if (Chosen.empty())
			{
				return String();
			}

			String BasePath = IsAbsolutePath(Chosen) ? String() : Path;
			MTexture Loaded = MTextureBuilder::Get().LoadTexture(BasePath, Chosen);
			if (Loaded.Width > 0 && Loaded.Height > 0)
			{
				const TextureType Slot = ToTextureType(Type);
				Loaded.bSRGB = IsSRGBTextureType(Slot);
				OutMat.Textures[Slot] = Loaded;
				OutMat.bValid = true;
				bLoaded = true;
				return Chosen;
			}

			cout << "FBX: Texture load failed (" << Label << ") " << NarrowString(Chosen) << endl;
			return String();
		};

		auto MakeSolidTexture = [](const wchar_t* Name, uint8 B, uint8 G, uint8 R, uint8 A) -> MTexture
		{
			MTexture T{};
			T.Name = Name;
			T.Width = 1;
			T.Height = 1;
			T.BitsPerPixel = 32;
			T.Size = 0;
			T.Pixels = { B, G, R, A };
			return T;
		};

		bool bDiffuseEmbedded = false;
		bool bDiffuseLoaded = false;
		bool bNormalEmbedded = false;
		bool bNormalLoaded = false;
		bool bSpecularEmbedded = false;
		bool bSpecularLoaded = false;
		bool bShininessEmbedded = false;
		bool bShininessLoaded = false;
		bool bAmbientEmbedded = false;
		bool bAmbientLoaded = false;
		bool bEmissiveEmbedded = false;
		bool bEmissiveLoaded = false;
		bool bReflectionEmbedded = false;
		bool bReflectionLoaded = false;

		const String DiffusePath = LoadTextureIfAny(ofbx::Texture::DIFFUSE, "diffuse", bDiffuseEmbedded, bDiffuseLoaded);
		if (!DiffusePath.empty())
		{
			OutMat.bBaseColor = 1;
		}

		const String NormalPath = LoadTextureIfAny(ofbx::Texture::NORMAL, "normal", bNormalEmbedded, bNormalLoaded);
		if (!NormalPath.empty())
		{
			OutMat.bNormal = 1;
		}

		const String SpecularPath = LoadTextureIfAny(ofbx::Texture::SPECULAR, "specular", bSpecularEmbedded, bSpecularLoaded);
		if (!SpecularPath.empty())
		{
			OutMat.bSpecular = 1;
		}

		const String ShininessPath = LoadTextureIfAny(ofbx::Texture::SHININESS, "shininess", bShininessEmbedded, bShininessLoaded);
		if (!ShininessPath.empty())
		{
			OutMat.bShininess = 1;
		}

		const String AmbientPath = LoadTextureIfAny(ofbx::Texture::AMBIENT, "ambient", bAmbientEmbedded, bAmbientLoaded);
		if (!AmbientPath.empty())
		{
			OutMat.bAmbient = 1;
		}

		const String EmissivePath = LoadTextureIfAny(ofbx::Texture::EMISSIVE, "emissive", bEmissiveEmbedded, bEmissiveLoaded);
		if (!EmissivePath.empty())
		{
			OutMat.bEmissive = 1;
		}

		const String ReflectionPath = LoadTextureIfAny(ofbx::Texture::REFLECTION, "reflection", bReflectionEmbedded, bReflectionLoaded);
		if (!ReflectionPath.empty())
		{
			OutMat.bReflection = 1;
		}

		if (!bDiffuseLoaded && bNormalLoaded)
		{
			OutMat.Textures[DIFFUSE] = MakeSolidTexture(TEXT("DefaultWhite"), 255, 255, 255, 255);
			OutMat.Textures[DIFFUSE].bSRGB = IsSRGBTextureType(DIFFUSE);
			OutMat.bBaseColor = 1;
			OutMat.bValid = true;
		}

		cout << "FBX: Material[" << NewIndex << "] "
			<< "Diffuse=" << (DiffusePath.empty() ? "none" : NarrowString(DiffusePath))
			<< (bDiffuseEmbedded ? " (embedded)" : "")
			<< " Normal=" << (NormalPath.empty() ? "none" : NarrowString(NormalPath))
			<< (bNormalEmbedded ? " (embedded)" : "")
			<< " Specular=" << (SpecularPath.empty() ? "none" : NarrowString(SpecularPath))
			<< (bSpecularEmbedded ? " (embedded)" : "")
			<< " Shininess=" << (ShininessPath.empty() ? "none" : NarrowString(ShininessPath))
			<< (bShininessEmbedded ? " (embedded)" : "")
			<< " Ambient=" << (AmbientPath.empty() ? "none" : NarrowString(AmbientPath))
			<< (bAmbientEmbedded ? " (embedded)" : "")
			<< " Emissive=" << (EmissivePath.empty() ? "none" : NarrowString(EmissivePath))
			<< (bEmissiveEmbedded ? " (embedded)" : "")
			<< " Reflection=" << (ReflectionPath.empty() ? "none" : NarrowString(ReflectionPath))
			<< (bReflectionEmbedded ? " (embedded)" : "")
			<< " Valid=" << (OutMat.bValid ? 1 : 0)
			<< endl;

		MaterialLookup.emplace(Material, NewIndex);
		return NewIndex;
	};

	const int MeshCount = Scene->getMeshCount();
	cout << "FBX: MeshCount = " << MeshCount << endl;

	bool bHasGlobalBounds = false;
	float3 GlobalMin{};
	float3 GlobalMax{};
	uint32 MeshesWithMaterialMap = 0;
	uint32 MeshesNoMaterialMap = 0;
	uint32 MeshesNoMapWithMaterial = 0;
	uint32 MeshesNoMapNoMaterial = 0;

	auto UpdateGlobalBounds = [&](const MMesh& Mesh)
	{
		if (Mesh.RenderData.Positions.empty())
		{
			return;
		}

		float3 Min = Mesh.RenderData.Positions[0];
		float3 Max = Mesh.RenderData.Positions[0];
		for (const auto& P : Mesh.RenderData.Positions)
		{
			Min.x = std::min(Min.x, P.x);
			Min.y = std::min(Min.y, P.y);
			Min.z = std::min(Min.z, P.z);
			Max.x = std::max(Max.x, P.x);
			Max.y = std::max(Max.y, P.y);
			Max.z = std::max(Max.z, P.z);
		}

		if (!bHasGlobalBounds)
		{
			GlobalMin = Min;
			GlobalMax = Max;
			bHasGlobalBounds = true;
		}
		else
		{
			GlobalMin.x = std::min(GlobalMin.x, Min.x);
			GlobalMin.y = std::min(GlobalMin.y, Min.y);
			GlobalMin.z = std::min(GlobalMin.z, Min.z);
			GlobalMax.x = std::max(GlobalMax.x, Max.x);
			GlobalMax.y = std::max(GlobalMax.y, Max.y);
			GlobalMax.z = std::max(GlobalMax.z, Max.z);
		}
	};

	for (int MeshIndex = 0; MeshIndex < MeshCount; ++MeshIndex)
	{
		const ofbx::Mesh* Mesh = Scene->getMesh(MeshIndex);
		if (!Mesh)
		{
			continue;
		}

		const ofbx::GeometryData& Geometry = Mesh->getGeometryData();
		if (!Geometry.hasVertices())
		{
			continue;
		}

		MMesh Out{};
		unordered_map<Vertex, uint32_t> UniqueVertices{};

		const ofbx::Vec3Attributes Positions = Geometry.getPositions();
		const ofbx::Vec3Attributes Normals = Geometry.getNormals();
		const ofbx::Vec2Attributes UVs = Geometry.getUVs(0);
		const ofbx::Vec2Attributes UVs1 = Geometry.getUVs(1);

		const bool bHasNormals = Normals.values != nullptr && Normals.count > 0;
		const bool bHasUVs = UVs.values != nullptr && UVs.count > 0;
		const bool bHasUVs1 = UVs1.values != nullptr && UVs1.count > 0;

		const int* MaterialMap = Geometry.getMaterialMap();
		const int MaterialMapSize = Geometry.getMaterialMapSize();
		const bool bHasMaterialMap = (MaterialMap != nullptr) && (MaterialMapSize > 0);
		const int MeshMaterialCount = Mesh->getMaterialCount();
		uint32 InvalidMaterialIndexCount = 0;
		uint32 NullMaterialPtrCount = 0;
		uint32 MaterialMapOobCount = 0;

		cout << "FBX: Mesh " << MeshIndex
			<< " Positions=" << Positions.count
			<< " Partitions=" << Geometry.getPartitionCount()
			<< " Normals=" << Normals.count
			<< " UVs=" << UVs.count
			<< " UVs1=" << UVs1.count
			<< " Materials=" << MeshMaterialCount
			<< " MaterialMap=" << (bHasMaterialMap ? "yes" : "no")
			<< " MaterialMapSize=" << MaterialMapSize
			<< endl;

		if (bHasMaterialMap)
		{
			++MeshesWithMaterialMap;
		}
		else
		{
			++MeshesNoMaterialMap;
			if (MeshMaterialCount > 0)
			{
				++MeshesNoMapWithMaterial;
				if (MeshMaterialCount > 1)
				{
					cout << "FBX: Mesh " << MeshIndex
						<< " has multiple materials but no material map; using material 0." << endl;
				}
			}
			else
			{
				++MeshesNoMapNoMaterial;
			}
		}

		const ofbx::DMatrix WorldD = MulDMatrix(Mesh->getGlobalTransform(), Mesh->getGeometricMatrix());
		SIMDMath::Matrix4x4 World = ToRowMajor(WorldD);

		bool bFlipWinding = false;
		const SIMDMath::Matrix4x4 AxisConversion = BuildAxisConversion(Scene->getGlobalSettings(), bFlipWinding);

		const float UnitScale = (Scene->getGlobalSettings()) ? Scene->getGlobalSettings()->UnitScaleFactor : 1.0f;
		const SIMDMath::Matrix4x4 UnitScaleM = SIMDMath::Matrix4x4::Scale(UnitScale, UnitScale, UnitScale);

		SIMDMath::Matrix4x4 WorldFinal = World * AxisConversion * UnitScaleM;
		const SIMDMath::Matrix4x4 NormalMatrix = SIMDMath::Matrix4x4::Transpose(SIMDMath::Matrix4x4::Inverse(WorldFinal));

		auto EmitTriangle = [&](int Index0, int Index1, int Index2)
		{
			int Indices[3] = { Index0, Index1, Index2 };
			if (bFlipWinding)
			{
				std::swap(Indices[1], Indices[2]);
			}

			Vertex V[3]{};
			for (int iTri = 0; iTri < 3; ++iTri)
			{
				const int Index = Indices[iTri];
				const ofbx::Vec3 P = Positions.get(Index);
				const SIMDMath::Vector3 Pos = SIMDMath::TransformPoint(
					SIMDMath::Vector3(static_cast<float>(P.x), static_cast<float>(P.y), static_cast<float>(P.z)),
					WorldFinal);
				V[iTri].position = float3{ Pos.x, Pos.y, Pos.z };

				if (bHasNormals)
				{
					const ofbx::Vec3 N = Normals.get(Index);
					const SIMDMath::Vector3 Nor = SIMDMath::TransformVector(
						SIMDMath::Vector3(static_cast<float>(N.x), static_cast<float>(N.y), static_cast<float>(N.z)),
						NormalMatrix);
					V[iTri].normal = Normalize(float3{ Nor.x, Nor.y, Nor.z });
				}

				if (bHasUVs)
				{
					const ofbx::Vec2 UV = UVs.get(Index);
					V[iTri].texCoord = float2{ UV.x, 1.0f - UV.y };
				}
				if (bHasUVs1)
				{
					const ofbx::Vec2 UV = UVs1.get(Index);
					V[iTri].texCoord1 = float2{ UV.x, 1.0f - UV.y };
				}
				else
				{
					V[iTri].texCoord1 = V[iTri].texCoord;
				}
			}

			if (!bHasNormals)
			{
				const float3 E0 = V[1].position - V[0].position;
				const float3 E1 = V[2].position - V[0].position;
				const float3 TriNormal = Normalize(Cross(E0, E1));
				for (int iTri = 0; iTri < 3; ++iTri)
				{
					V[iTri].normal = TriNormal;
				}
			}

			float3 TriTangent{ 1.0f, 0.0f, 0.0f };
			float3 TriBitangent{ 0.0f, 1.0f, 0.0f };
			if (bHasUVs)
			{
				const float3 E0 = V[1].position - V[0].position;
				const float3 E1 = V[2].position - V[0].position;
				const float2 dUV0 = V[1].texCoord - V[0].texCoord;
				const float2 dUV1 = V[2].texCoord - V[0].texCoord;

				const float Det = dUV0.x * dUV1.y - dUV0.y * dUV1.x;
				if (Det != 0.0f)
				{
					const float F = 1.0f / Det;
					TriTangent = float3
					{
						F * ((dUV1.y * E0.x) + (-dUV0.y * E1.x)),
						F * ((dUV1.y * E0.y) + (-dUV0.y * E1.y)),
						F * ((dUV1.y * E0.z) + (-dUV0.y * E1.z))
					};

					TriBitangent = float3
					{
						F * ((-dUV1.x * E0.x) + (dUV0.x * E1.x)),
						F * ((-dUV1.x * E0.y) + (dUV0.x * E1.y)),
						F * ((-dUV1.x * E0.z) + (dUV0.x * E1.z))
					};

					TriTangent = Normalize(TriTangent);
					TriBitangent = Normalize(TriBitangent);
				}
			}
			{
				const SIMDMath::Vector3 Tan = SIMDMath::TransformVector(
					SIMDMath::Vector3(TriTangent.x, TriTangent.y, TriTangent.z),
					NormalMatrix);
				const SIMDMath::Vector3 Bit = SIMDMath::TransformVector(
					SIMDMath::Vector3(TriBitangent.x, TriBitangent.y, TriBitangent.z),
					NormalMatrix);
				TriTangent = Normalize(float3{ Tan.x, Tan.y, Tan.z });
				TriBitangent = Normalize(float3{ Bit.x, Bit.y, Bit.z });
			}

			for (int iTri = 0; iTri < 3; ++iTri)
			{
				V[iTri].tangent = TriTangent;
				V[iTri].bitangent = TriBitangent;
			}

			for (int iTri = 0; iTri < 3; ++iTri)
			{
				if (UniqueVertices.count(V[iTri]) == 0)
				{
					UniqueVertices[V[iTri]] = static_cast<uint32_t>(Out.RenderData.Positions.size());
					Out.RenderData.Positions.push_back(V[iTri].position);
					Out.RenderData.GetUVChannel(0).push_back(V[iTri].texCoord);
					if (bHasUVs1)
					{
						Out.RenderData.GetUVChannel(1).push_back(V[iTri].texCoord1);
					}
					Out.RenderData.Normals.push_back(V[iTri].normal);
					Out.RenderData.Tangents.push_back(V[iTri].tangent);
					Out.RenderData.Bitangents.push_back(V[iTri].bitangent);
				}
			}

			Out.RenderData.Indices.push_back(UniqueVertices[V[0]]);
			Out.RenderData.Indices.push_back(UniqueVertices[V[1]]);
			Out.RenderData.Indices.push_back(UniqueVertices[V[2]]);
		};

		auto GetMaterialIndexSafe = [&](int MatIndex) -> uint32
		{
			if (MatIndex < 0 || MatIndex >= MeshMaterialCount)
			{
				++InvalidMaterialIndexCount;
				if (InvalidMaterialIndexCount <= 10)
				{
					cout << "FBX: Mesh " << MeshIndex << " invalid material index " << MatIndex
						<< " (materialCount=" << MeshMaterialCount << ")" << endl;
				}
				assert(false && "FBX: invalid material index");
				return 0;
			}
			const ofbx::Material* Material = Mesh->getMaterial(MatIndex);
			if (!Material)
			{
				++NullMaterialPtrCount;
				if (NullMaterialPtrCount <= 10)
				{
					cout << "FBX: Mesh " << MeshIndex << " null material pointer at index " << MatIndex << endl;
				}
				assert(false && "FBX: null material pointer");
				return 0;
			}
			return GetMaterialIndex(Material);
		};

		int GlobalPolygonIndex = 0;
		const int PartitionCount = Geometry.getPartitionCount();
		const bool bUsePartitionMaterial = (!bHasMaterialMap) && (PartitionCount > 1) && (MeshMaterialCount > 0);
		const bool bHasMeshMaterial = (MeshMaterialCount > 0);
		const uint32 InvalidMaterial = static_cast<uint32>(-1);

		auto GetPolygonMaterial = [&](int PolygonIndex, int PartitionIndex) -> uint32
		{
			if (bHasMaterialMap)
			{
				if (PolygonIndex < 0 || PolygonIndex >= MaterialMapSize)
				{
					++MaterialMapOobCount;
					if (MaterialMapOobCount <= 10)
					{
						cout << "FBX: Mesh " << MeshIndex << " material map out of range "
							<< PolygonIndex << " (size=" << MaterialMapSize << ")" << endl;
					}
					assert(false && "FBX: material map index out of range");
					return 0;
				}
				return GetMaterialIndexSafe(MaterialMap[PolygonIndex]);
			}
			if (bUsePartitionMaterial)
			{
				return GetMaterialIndexSafe(PartitionIndex);
			}
			if (bHasMeshMaterial)
			{
				return GetMaterialIndexSafe(0);
			}
			return 0;
		};

		for (int PartitionIndex = 0; PartitionIndex < PartitionCount; ++PartitionIndex)
		{
			const ofbx::GeometryPartition Partition = Geometry.getPartition(PartitionIndex);
			size_t SectionStart = Out.RenderData.Indices.size();
			uint32 CurrentMaterial = InvalidMaterial;

			for (int PolygonIndex = 0; PolygonIndex < Partition.polygon_count; ++PolygonIndex, ++GlobalPolygonIndex)
			{
				const uint32 PolygonMaterial = GetPolygonMaterial(GlobalPolygonIndex, PartitionIndex);
				if (CurrentMaterial == InvalidMaterial)
				{
					CurrentMaterial = PolygonMaterial;
				}
				else if (PolygonMaterial != CurrentMaterial)
				{
					const size_t SectionEnd = Out.RenderData.Indices.size();
					if (SectionEnd > SectionStart)
					{
						Out.Sections.push_back(MSectionData{
							static_cast<uint32>(SectionStart),
							static_cast<uint32>(SectionEnd),
							CurrentMaterial });
					}
					SectionStart = Out.RenderData.Indices.size();
					CurrentMaterial = PolygonMaterial;
				}

				const ofbx::GeometryPartition::Polygon Polygon = Partition.polygons[PolygonIndex];
				const int VertexCount = Polygon.vertex_count;
				if (VertexCount < 3)
				{
					continue;
				}

				for (int Tri = 1; Tri + 1 < VertexCount; ++Tri)
				{
					EmitTriangle(
						Polygon.from_vertex,
						Polygon.from_vertex + Tri,
						Polygon.from_vertex + Tri + 1);
				}
			}

			const size_t SectionEnd = Out.RenderData.Indices.size();
			if (SectionEnd > SectionStart)
			{
				if (CurrentMaterial == InvalidMaterial)
				{
					CurrentMaterial = 0;
				}
				Out.Sections.push_back(MSectionData{
					static_cast<uint32>(SectionStart),
					static_cast<uint32>(SectionEnd),
					CurrentMaterial });
			}
		}

		if (PartitionCount == 0)
		{
			const int PositionCount = Positions.count;
			size_t SectionStart = Out.RenderData.Indices.size();
			uint32 CurrentMaterial = InvalidMaterial;

			if (PositionCount >= 3)
			{
				for (int i = 0; i + 2 < PositionCount; i += 3)
				{
					const uint32 PolygonMaterial = GetPolygonMaterial(GlobalPolygonIndex, 0);
					if (CurrentMaterial == InvalidMaterial)
					{
						CurrentMaterial = PolygonMaterial;
					}
					else if (PolygonMaterial != CurrentMaterial)
					{
						const size_t SectionEnd = Out.RenderData.Indices.size();
						if (SectionEnd > SectionStart)
						{
							Out.Sections.push_back(MSectionData{
								static_cast<uint32>(SectionStart),
								static_cast<uint32>(SectionEnd),
								CurrentMaterial });
						}
						SectionStart = Out.RenderData.Indices.size();
						CurrentMaterial = PolygonMaterial;
					}
					EmitTriangle(i, i + 1, i + 2);
					++GlobalPolygonIndex;
				}
			}
			else
			{
				cout << "FBX: Mesh " << MeshIndex << " has no partitions and no triangle data." << endl;
			}

			const size_t SectionEnd = Out.RenderData.Indices.size();
			if (SectionEnd > SectionStart)
			{
				if (CurrentMaterial == InvalidMaterial)
				{
					CurrentMaterial = 0;
				}
				Out.Sections.push_back(MSectionData{
					static_cast<uint32>(SectionStart),
					static_cast<uint32>(SectionEnd),
					CurrentMaterial });
			}
		}

		UpdateGlobalBounds(Out);
		OutMeshes.emplace_back(std::move(Out));

		if (InvalidMaterialIndexCount > 0 || NullMaterialPtrCount > 0 || MaterialMapOobCount > 0)
		{
			cout << "FBX: Mesh " << MeshIndex << " material issues: invalidIndex="
				<< InvalidMaterialIndexCount << " nullPtr=" << NullMaterialPtrCount
				<< " mapOob=" << MaterialMapOobCount << endl;
		}
	}

	if (bHasGlobalBounds)
	{
		MMesh AxisMesh{};
		const float3 Extent = GlobalMax - GlobalMin;
		float AxisLength = std::max(Extent.x, std::max(Extent.y, Extent.z)) * 0.35f;
		if (AxisLength < 1.0f)
		{
			AxisLength = 1.0f;
		}
		const float Thickness = std::max(0.02f, AxisLength * 0.05f);

		const float3 Center = (GlobalMin + GlobalMax) * 0.5f;
		const auto AxisMaterials = EnsureAxisMaterials(OutMaterials);

		auto AppendAxisSet = [&](const float3& Origin)
		{
			AppendAxisBox(AxisMesh,
				float3{ Origin.x, Origin.y - Thickness, Origin.z - Thickness },
				float3{ Origin.x + AxisLength, Origin.y + Thickness, Origin.z + Thickness },
				AxisMaterials[0]);

			AppendAxisBox(AxisMesh,
				float3{ Origin.x - Thickness, Origin.y, Origin.z - Thickness },
				float3{ Origin.x + Thickness, Origin.y + AxisLength, Origin.z + Thickness },
				AxisMaterials[1]);

			AppendAxisBox(AxisMesh,
				float3{ Origin.x - Thickness, Origin.y - Thickness, Origin.z },
				float3{ Origin.x + Thickness, Origin.y + Thickness, Origin.z + AxisLength },
				AxisMaterials[2]);
		};

		AppendAxisSet(float3{ 0.0f, 0.0f, 0.0f });     // World axis
		AppendAxisSet(Center);                         // Local axis (mesh center)

		OutMeshes.emplace_back(std::move(AxisMesh));
	}

	if (OutMaterials.empty())
	{
		OutMaterials.emplace_back();
	}

	cout << "FBX: MaterialMap summary: withMap=" << MeshesWithMaterialMap
		<< " noMapWithMaterial=" << MeshesNoMapWithMaterial
		<< " noMapNoMaterial=" << MeshesNoMapNoMaterial
		<< " total=" << MeshCount << endl;

	Scene->destroy();

	return OutMeshes;
}
