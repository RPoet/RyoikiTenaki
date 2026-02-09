#include <array>
#include <unordered_map>
#include <cmath>

#include "MeshBuilder.h"
#include "../../Engine.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

static float2 Subtract(const float2& RHS, const float2& LHS)
{
	return float2{ RHS.x - LHS.x, RHS.y - LHS.y };
}

static float3 Subtract(const float3& RHS, const float3& LHS)
{
	return float3{ RHS.x - LHS.x, RHS.y - LHS.y, RHS.z - LHS.z };
}

static float3 Scale(const float3& Vector, float Scalar)
{
	return float3{ Vector.x * Scalar, Vector.y * Scalar, Vector.z * Scalar };
}

static float InnerProduct(const float3& RHS, const float3& LHS)
{
	return RHS.x * LHS.x + RHS.y * LHS.y + RHS.z * LHS.z;
}

static float Distance(const float3& Vector)
{
	return std::sqrt(InnerProduct(Vector, Vector));
}

static float3 Normalize(const float3& Vector)
{
	const float Len = Distance(Vector);
	if (Len != 0.0f)
	{
		return float3{ Vector.x / Len, Vector.y / Len, Vector.z / Len };
	}
	return float3(0, 0, 0);
}

static float3 Cross(const float3& RHS, const float3& LHS)
{
	return float3
	{
		RHS.y * LHS.z - RHS.z * LHS.y,
		RHS.z * LHS.x - RHS.x * LHS.z,
		RHS.x * LHS.y - RHS.y * LHS.x,
	};
}

struct Vertex
{
	float3 position;
	float3 normal;
	float3 tangent;
	float3 bitangent;
	float2 texCoord;
	float2 texCoord1;

	bool operator==(const Vertex& Other) const
	{
		return position == Other.position
			&& normal == Other.normal
			&& tangent == Other.tangent
			&& bitangent == Other.bitangent
			&& texCoord == Other.texCoord
			&& texCoord1 == Other.texCoord1;
	}
};

namespace std
{
	template<> struct hash<float3>
	{
		size_t operator()(const float3& v) const
		{
			return ((hash<float>()(v.x) ^ (hash<float>()(v.y) << 1)) >> 1) ^ (hash<float>()(v.z) << 1);
		}
	};

	template<> struct hash<float2>
	{
		size_t operator()(const float2& v) const
		{
			return (std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1));
		}
	};

	template<> struct hash<Vertex>
	{
		size_t operator()(const Vertex& vertex) const
		{
			return ((((((hash<float3>()(vertex.position) ^
				(hash<float3>()(vertex.normal) << 1)) >> 1) ^
				(hash<float2>()(vertex.texCoord) << 1)) >> 1) ^
				(hash<float3>()(vertex.tangent) << 1)) >> 1) ^
				(hash<float3>()(vertex.bitangent) << 1) ^
				(hash<float2>()(vertex.texCoord1) << 2);
		}
	};
}

static bool ReadVec2Accessor(const tinygltf::Model& Model, int AccessorIndex, vector<float2>& Out)
{
	if (AccessorIndex < 0)
	{
		return false;
	}

	const auto& Accessor = Model.accessors[AccessorIndex];
	if (Accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.type != TINYGLTF_TYPE_VEC2)
	{
		return false;
	}

	const auto& BufferView = Model.bufferViews[Accessor.bufferView];
	const auto& Buffer = Model.buffers[BufferView.buffer];
	const unsigned char* Data = Buffer.data.data() + BufferView.byteOffset + Accessor.byteOffset;
	size_t Stride = Accessor.ByteStride(BufferView);
	if (Stride == 0)
	{
		Stride = sizeof(float) * 2;
	}

	Out.resize(Accessor.count);
	for (size_t i = 0; i < Accessor.count; ++i)
	{
		const float* Vec = reinterpret_cast<const float*>(Data + Stride * i);
		Out[i] = float2{ Vec[0], Vec[1] };
	}

	return true;
}

static bool ReadVec3Accessor(const tinygltf::Model& Model, int AccessorIndex, vector<float3>& Out)
{
	if (AccessorIndex < 0)
	{
		return false;
	}

	const auto& Accessor = Model.accessors[AccessorIndex];
	if (Accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.type != TINYGLTF_TYPE_VEC3)
	{
		return false;
	}

	const auto& BufferView = Model.bufferViews[Accessor.bufferView];
	const auto& Buffer = Model.buffers[BufferView.buffer];
	const unsigned char* Data = Buffer.data.data() + BufferView.byteOffset + Accessor.byteOffset;
	size_t Stride = Accessor.ByteStride(BufferView);
	if (Stride == 0)
	{
		Stride = sizeof(float) * 3;
	}

	Out.resize(Accessor.count);
	for (size_t i = 0; i < Accessor.count; ++i)
	{
		const float* Vec = reinterpret_cast<const float*>(Data + Stride * i);
		Out[i] = float3{ Vec[0], Vec[1], Vec[2] };
	}

	return true;
}

static bool ReadVec4Accessor(const tinygltf::Model& Model, int AccessorIndex, vector<float4>& Out)
{
	if (AccessorIndex < 0)
	{
		return false;
	}

	const auto& Accessor = Model.accessors[AccessorIndex];
	if (Accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.type != TINYGLTF_TYPE_VEC4)
	{
		return false;
	}

	const auto& BufferView = Model.bufferViews[Accessor.bufferView];
	const auto& Buffer = Model.buffers[BufferView.buffer];
	const unsigned char* Data = Buffer.data.data() + BufferView.byteOffset + Accessor.byteOffset;
	size_t Stride = Accessor.ByteStride(BufferView);
	if (Stride == 0)
	{
		Stride = sizeof(float) * 4;
	}

	Out.resize(Accessor.count);
	for (size_t i = 0; i < Accessor.count; ++i)
	{
		const float* Vec = reinterpret_cast<const float*>(Data + Stride * i);
		Out[i] = float4{ Vec[0], Vec[1], Vec[2], Vec[3] };
	}

	return true;
}

static void ReadIndicesAccessor(const tinygltf::Model& Model, int AccessorIndex, vector<uint32>& Out)
{
	Out.clear();
	if (AccessorIndex < 0)
	{
		return;
	}

	const auto& Accessor = Model.accessors[AccessorIndex];
	if (Accessor.type != TINYGLTF_TYPE_SCALAR)
	{
		return;
	}

	const auto& BufferView = Model.bufferViews[Accessor.bufferView];
	const auto& Buffer = Model.buffers[BufferView.buffer];
	const unsigned char* Data = Buffer.data.data() + BufferView.byteOffset + Accessor.byteOffset;
	size_t Stride = Accessor.ByteStride(BufferView);
	if (Stride == 0)
	{
		Stride = tinygltf::GetComponentSizeInBytes(Accessor.componentType);
	}

	Out.resize(Accessor.count);
	for (size_t i = 0; i < Accessor.count; ++i)
	{
		const unsigned char* Ptr = Data + Stride * i;
		switch (Accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
			Out[i] = *reinterpret_cast<const uint8*>(Ptr);
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			Out[i] = *reinterpret_cast<const uint16*>(Ptr);
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			Out[i] = *reinterpret_cast<const uint32*>(Ptr);
			break;
		default:
			Out[i] = 0;
			break;
		}
	}
}

static MTexture BuildTextureFromImage(const tinygltf::Image& Image, const String& FallbackName)
{
	MTexture Texture{};
	Texture.Width = Image.width;
	Texture.Height = Image.height;
	Texture.BitsPerPixel = 32;
	Texture.Size = static_cast<uint32>(Texture.Width * Texture.Height * 4);

	const std::string& RawName = Image.uri.empty() ? Image.name : Image.uri;
	if (!RawName.empty())
	{
		Texture.Name = String(RawName.begin(), RawName.end());
	}
	else
	{
		Texture.Name = FallbackName;
	}

	if (Texture.Width == 0 || Texture.Height == 0 || Image.image.empty())
	{
		return Texture;
	}

	const size_t PixelCount = static_cast<size_t>(Texture.Width * Texture.Height);
	Texture.Pixels.resize(PixelCount * 4);

	const int Components = Image.component;
	const unsigned char* Src = Image.image.data();

	for (size_t i = 0; i < PixelCount; ++i)
	{
		uint8 r = 255;
		uint8 g = 255;
		uint8 b = 255;
		uint8 a = 255;

		const size_t Offset = static_cast<size_t>(i * Components);
		if (Components == 1)
		{
			r = Src[Offset + 0];
			g = r;
			b = r;
		}
		else if (Components == 2)
		{
			r = Src[Offset + 0];
			g = r;
			b = r;
			a = Src[Offset + 1];
		}
		else
		{
			r = Src[Offset + 0];
			g = Src[Offset + 1];
			b = Src[Offset + 2];
			if (Components >= 4)
			{
				a = Src[Offset + 3];
			}
		}

		const size_t Dst = i * 4;
		Texture.Pixels[Dst + 0] = b;
		Texture.Pixels[Dst + 1] = g;
		Texture.Pixels[Dst + 2] = r;
		Texture.Pixels[Dst + 3] = a;
	}

	return Texture;
}

static MTexture MakeSolidTexture(const String& Name, uint8 R, uint8 G, uint8 B, uint8 A)
{
	MTexture Texture{};
	Texture.Name = Name;
	Texture.Width = 1;
	Texture.Height = 1;
	Texture.BitsPerPixel = 32;
	Texture.Size = 4;
	Texture.Pixels = { B, G, R, A };
	return Texture;
}

static int GetAttributeAccessorIndex(const tinygltf::Primitive& Primitive, const std::string& Name)
{
	auto It = Primitive.attributes.find(Name);
	if (It == Primitive.attributes.end())
	{
		return -1;
	}
	return It->second;
}

vector<MMesh> MMeshBuilder::LoadMeshGLTF(const String& Path, const String& ModelName, vector<Material>& OutMaterials)
{
	vector<MMesh> OutMeshes;
	OutMaterials.clear();

	const String Combined = Path + ModelName;
	const std::string FilePath(Combined.begin(), Combined.end());

	tinygltf::TinyGLTF Loader;
	tinygltf::Model Model;
	std::string Err;
	std::string Warn;

	bool bLoaded = false;
	if (Combined.size() >= 4)
	{
		const String Ext = Combined.substr(Combined.size() - 4);
		if (Ext == TEXT(".glb") || Ext == TEXT(".GLB"))
		{
			bLoaded = Loader.LoadBinaryFromFile(&Model, &Err, &Warn, FilePath);
		}
		else
		{
			bLoaded = Loader.LoadASCIIFromFile(&Model, &Err, &Warn, FilePath);
		}
	}

	if (!Warn.empty())
	{
		cout << Warn << endl;
	}

	if (!bLoaded)
	{
		cout << Err << endl;
		return OutMeshes;
	}

	if (Model.materials.empty())
	{
		OutMaterials.emplace_back();
	}
	else
	{
		OutMaterials.resize(Model.materials.size());
		for (size_t i = 0; i < Model.materials.size(); ++i)
		{
			const auto& SourceMaterial = Model.materials[i];
			Material& OutMaterial = OutMaterials[i];
			OutMaterial = Material{};
			OutMaterial.Textures.resize(TextureType::COUNT);
			auto AssignTexture = [&](TextureType Type, const MTexture& Texture)
			{
				OutMaterial.Textures[Type] = Texture;
				OutMaterial.Textures[Type].bSRGB = IsSRGBTextureType(Type);
			};

			const auto& Pbr = SourceMaterial.pbrMetallicRoughness;
			float4 BaseColor = float4(1, 1, 1, 1);
			if (Pbr.baseColorFactor.size() == 4)
			{
				BaseColor = float4(
					static_cast<float>(Pbr.baseColorFactor[0]),
					static_cast<float>(Pbr.baseColorFactor[1]),
					static_cast<float>(Pbr.baseColorFactor[2]),
					static_cast<float>(Pbr.baseColorFactor[3]));
			}

			bool bHasBaseColor = false;
			bool bHasNormal = false;
			bool bHasMetallicRoughness = false;
			bool bHasOcclusion = false;
			bool bHasEmissive = false;
			OutMaterial.BaseColorUVIndex = 0;
			OutMaterial.NormalUVIndex = 0;
			OutMaterial.MetallicRoughnessUVIndex = 0;
			OutMaterial.AmbientUVIndex = 0;
			OutMaterial.EmissiveUVIndex = 0;
			OutMaterial.ShininessUVIndex = 0;

			if (Pbr.baseColorTexture.index >= 0 && Pbr.baseColorTexture.index < static_cast<int>(Model.textures.size()))
			{
				const auto& Texture = Model.textures[Pbr.baseColorTexture.index];
				if (Texture.source >= 0 && Texture.source < static_cast<int>(Model.images.size()))
				{
					const auto& Image = Model.images[Texture.source];
					AssignTexture(DIFFUSE, BuildTextureFromImage(Image, TEXT("BaseColor")));
					bHasBaseColor = true;
					OutMaterial.BaseColorUVIndex = (Pbr.baseColorTexture.texCoord > 0)
						? static_cast<uint32>(Pbr.baseColorTexture.texCoord)
						: 0u;
				}
			}

			if (SourceMaterial.normalTexture.index >= 0 && SourceMaterial.normalTexture.index < static_cast<int>(Model.textures.size()))
			{
				const auto& Texture = Model.textures[SourceMaterial.normalTexture.index];
				if (Texture.source >= 0 && Texture.source < static_cast<int>(Model.images.size()))
				{
					const auto& Image = Model.images[Texture.source];
					AssignTexture(NORMAL, BuildTextureFromImage(Image, TEXT("Normal")));
					bHasNormal = true;
					OutMaterial.NormalUVIndex = (SourceMaterial.normalTexture.texCoord > 0)
						? static_cast<uint32>(SourceMaterial.normalTexture.texCoord)
						: 0u;
				}
			}

			if (Pbr.metallicRoughnessTexture.index >= 0 && Pbr.metallicRoughnessTexture.index < static_cast<int>(Model.textures.size()))
			{
				const auto& Texture = Model.textures[Pbr.metallicRoughnessTexture.index];
				if (Texture.source >= 0 && Texture.source < static_cast<int>(Model.images.size()))
				{
					const auto& Image = Model.images[Texture.source];
					AssignTexture(SPECULAR, BuildTextureFromImage(Image, TEXT("MetallicRoughness")));
					bHasMetallicRoughness = true;
					OutMaterial.bMetallicRoughness = 1;
					OutMaterial.MetallicRoughnessUVIndex = (Pbr.metallicRoughnessTexture.texCoord > 0)
						? static_cast<uint32>(Pbr.metallicRoughnessTexture.texCoord)
						: 0u;
				}
			}

			if (SourceMaterial.occlusionTexture.index >= 0 && SourceMaterial.occlusionTexture.index < static_cast<int>(Model.textures.size()))
			{
				const auto& Texture = Model.textures[SourceMaterial.occlusionTexture.index];
				if (Texture.source >= 0 && Texture.source < static_cast<int>(Model.images.size()))
				{
					const auto& Image = Model.images[Texture.source];
					AssignTexture(AMBIENT, BuildTextureFromImage(Image, TEXT("Occlusion")));
					bHasOcclusion = true;
					OutMaterial.AmbientUVIndex = (SourceMaterial.occlusionTexture.texCoord > 0)
						? static_cast<uint32>(SourceMaterial.occlusionTexture.texCoord)
						: 0u;
				}
			}

			if (SourceMaterial.emissiveTexture.index >= 0 && SourceMaterial.emissiveTexture.index < static_cast<int>(Model.textures.size()))
			{
				const auto& Texture = Model.textures[SourceMaterial.emissiveTexture.index];
				if (Texture.source >= 0 && Texture.source < static_cast<int>(Model.images.size()))
				{
					const auto& Image = Model.images[Texture.source];
					AssignTexture(EMISSIVE, BuildTextureFromImage(Image, TEXT("Emissive")));
					bHasEmissive = true;
					OutMaterial.EmissiveUVIndex = (SourceMaterial.emissiveTexture.texCoord > 0)
						? static_cast<uint32>(SourceMaterial.emissiveTexture.texCoord)
						: 0u;
				}
			}

			if (!bHasBaseColor && bHasNormal)
			{
				AssignTexture(DIFFUSE, MakeSolidTexture(TEXT("DefaultWhite"), 255, 255, 255, 255));
				bHasBaseColor = true;
			}

			if (OutMaterial.BaseColorUVIndex > 1 || OutMaterial.NormalUVIndex > 1 ||
				OutMaterial.MetallicRoughnessUVIndex > 1 || OutMaterial.AmbientUVIndex > 1 ||
				OutMaterial.EmissiveUVIndex > 1)
			{
				cout << "GLTF: Material " << i << " uses UV set > 1 (BaseColorUV="
					<< OutMaterial.BaseColorUVIndex << ", NormalUV=" << OutMaterial.NormalUVIndex
					<< ", MetallicRoughnessUV=" << OutMaterial.MetallicRoughnessUVIndex
					<< ", AmbientUV=" << OutMaterial.AmbientUVIndex
					<< ", EmissiveUV=" << OutMaterial.EmissiveUVIndex
					<< "). Only TEXCOORD_0/1 are supported." << endl;
			}

			if (BaseColor.x != 1.0f || BaseColor.y != 1.0f || BaseColor.z != 1.0f)
			{
				OutMaterial.Colors.emplace_back(BaseColor.x, BaseColor.y, BaseColor.z);
			}

			OutMaterial.bBaseColor = bHasBaseColor;
			OutMaterial.bNormal = bHasNormal;
			OutMaterial.bSpecular = bHasMetallicRoughness ? 1u : 0u;
			OutMaterial.bAmbient = bHasOcclusion ? 1u : 0u;
			OutMaterial.bEmissive = bHasEmissive ? 1u : 0u;
			OutMaterial.bValid = bHasBaseColor || bHasNormal || bHasMetallicRoughness || bHasOcclusion || bHasEmissive || !OutMaterial.Colors.empty();

			cout << "GLTF: Material[" << i << "] "
				<< "BaseColor=" << (bHasBaseColor ? "yes" : "no")
				<< " Normal=" << (bHasNormal ? "yes" : "no")
				<< " MetallicRoughness=" << (bHasMetallicRoughness ? "yes" : "no")
				<< " Occlusion=" << (bHasOcclusion ? "yes" : "no")
				<< " Emissive=" << (bHasEmissive ? "yes" : "no")
				<< endl;
		}
	}

	for (const auto& Mesh : Model.meshes)
	{
		MMesh Out{};
		unordered_map<Vertex, uint32> UniqueVertices{};

		for (const auto& Primitive : Mesh.primitives)
		{
			if (Primitive.mode != TINYGLTF_MODE_TRIANGLES)
			{
				continue;
			}

			const int PositionIndex = GetAttributeAccessorIndex(Primitive, "POSITION");
			vector<float3> Positions;
			if (!ReadVec3Accessor(Model, PositionIndex, Positions))
			{
				continue;
			}

			vector<float3> Normals;
			bool bHasNormals = ReadVec3Accessor(Model, GetAttributeAccessorIndex(Primitive, "NORMAL"), Normals);

			vector<float2> TexCoords;
			bool bHasTexCoord = ReadVec2Accessor(Model, GetAttributeAccessorIndex(Primitive, "TEXCOORD_0"), TexCoords);

			vector<float2> TexCoords1;
			bool bHasTexCoord1 = ReadVec2Accessor(Model, GetAttributeAccessorIndex(Primitive, "TEXCOORD_1"), TexCoords1);

			vector<float4> Tangents;
			bool bHasTangents = ReadVec4Accessor(Model, GetAttributeAccessorIndex(Primitive, "TANGENT"), Tangents);

			if (bHasNormals && Normals.size() != Positions.size())
			{
				cout << "GLTF: Normal count mismatch, ignoring normals." << endl;
				bHasNormals = false;
			}
			if (bHasTexCoord && TexCoords.size() != Positions.size())
			{
				cout << "GLTF: Texcoord count mismatch, ignoring texcoords." << endl;
				bHasTexCoord = false;
			}
			if (bHasTexCoord1 && TexCoords1.size() != Positions.size())
			{
				cout << "GLTF: Texcoord1 count mismatch, ignoring texcoords." << endl;
				bHasTexCoord1 = false;
			}
			if (bHasTangents && Tangents.size() != Positions.size())
			{
				cout << "GLTF: Tangent count mismatch, ignoring tangents." << endl;
				bHasTangents = false;
			}

			vector<uint32> Indices;
			ReadIndicesAccessor(Model, Primitive.indices, Indices);
			if (Indices.empty())
			{
				Indices.resize(Positions.size());
				for (size_t i = 0; i < Positions.size(); ++i)
				{
					Indices[i] = static_cast<uint32>(i);
				}
			}

			const size_t IndexStart = Out.RenderData.Indices.size();
			size_t OutOfRangeCount = 0;

			for (size_t i = 0; i + 2 < Indices.size(); i += 3)
			{
				const uint32 I0 = Indices[i];
				const uint32 I1 = Indices[i + 1];
				const uint32 I2 = Indices[i + 2];

				if (I0 >= Positions.size() || I1 >= Positions.size() || I2 >= Positions.size())
				{
					++OutOfRangeCount;
					continue;
				}

				Vertex V[3]{};
				V[0].position = Positions[I0];
				V[1].position = Positions[I1];
				V[2].position = Positions[I2];

				if (bHasNormals)
				{
					V[0].normal = Normals[I0];
					V[1].normal = Normals[I1];
					V[2].normal = Normals[I2];
				}

				if (bHasTexCoord)
				{
					V[0].texCoord = TexCoords[I0];
					V[1].texCoord = TexCoords[I1];
					V[2].texCoord = TexCoords[I2];
				}
				if (bHasTexCoord1)
				{
					V[0].texCoord1 = TexCoords1[I0];
					V[1].texCoord1 = TexCoords1[I1];
					V[2].texCoord1 = TexCoords1[I2];
				}
				else
				{
					V[0].texCoord1 = V[0].texCoord;
					V[1].texCoord1 = V[1].texCoord;
					V[2].texCoord1 = V[2].texCoord;
				}

				if (bHasTangents)
				{
					const float4 T0 = Tangents[I0];
					const float4 T1 = Tangents[I1];
					const float4 T2 = Tangents[I2];
					V[0].tangent = float3{ T0.x, T0.y, T0.z };
					V[1].tangent = float3{ T1.x, T1.y, T1.z };
					V[2].tangent = float3{ T2.x, T2.y, T2.z };
				}

				if (!bHasNormals)
				{
					const float3 E0 = Subtract(V[1].position, V[0].position);
					const float3 E1 = Subtract(V[2].position, V[0].position);
					const float3 TriNormal = Normalize(Cross(E0, E1));

					V[0].normal = TriNormal;
					V[1].normal = TriNormal;
					V[2].normal = TriNormal;
				}

				if (!bHasTangents)
				{
					float3 TriTangent{ 1.0f, 0.0f, 0.0f };
					float3 TriBitangent{ 0.0f, 1.0f, 0.0f };

					if (bHasTexCoord)
					{
						const float3 E0 = Subtract(V[1].position, V[0].position);
						const float3 E1 = Subtract(V[2].position, V[0].position);
						const float2 dUV0 = Subtract(V[1].texCoord, V[0].texCoord);
						const float2 dUV1 = Subtract(V[2].texCoord, V[0].texCoord);

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

					for (int32 iTri = 0; iTri < 3; ++iTri)
					{
						V[iTri].tangent = TriTangent;
						V[iTri].bitangent = TriBitangent;
					}
				}
				else
				{
					const float Sign0 = (Tangents[I0].w < 0.0f) ? -1.0f : 1.0f;
					const float Sign1 = (Tangents[I1].w < 0.0f) ? -1.0f : 1.0f;
					const float Sign2 = (Tangents[I2].w < 0.0f) ? -1.0f : 1.0f;

					const float3 Bitangent0 = Scale(Normalize(Cross(V[0].normal, V[0].tangent)), Sign0);
					const float3 Bitangent1 = Scale(Normalize(Cross(V[1].normal, V[1].tangent)), Sign1);
					const float3 Bitangent2 = Scale(Normalize(Cross(V[2].normal, V[2].tangent)), Sign2);
					V[0].bitangent = Bitangent0;
					V[1].bitangent = Bitangent1;
					V[2].bitangent = Bitangent2;
				}

				for (int32 iTri = 0; iTri < 3; ++iTri)
				{
					if (UniqueVertices.count(V[iTri]) == 0)
					{
						UniqueVertices[V[iTri]] = static_cast<uint32>(Out.RenderData.Positions.size());
						Out.RenderData.Positions.push_back(V[iTri].position);
						Out.RenderData.GetUVChannel(0).push_back(V[iTri].texCoord);
						if (bHasTexCoord1)
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
			}

			if (OutOfRangeCount > 0)
			{
				cout << "GLTF: " << OutOfRangeCount << " triangles skipped due to out-of-range indices." << endl;
			}

			const size_t IndexEnd = Out.RenderData.Indices.size();
			int32 MaterialId = Primitive.material;
			if (MaterialId < 0 || MaterialId >= static_cast<int32>(OutMaterials.size()))
			{
				MaterialId = 0;
			}

			MSectionData Section{ static_cast<uint32>(IndexStart), static_cast<uint32>(IndexEnd), static_cast<uint32>(MaterialId) };
			Out.Sections.push_back(Section);
		}

		if (!Out.RenderData.Positions.empty() || !Out.RenderData.Indices.empty())
		{
			OutMeshes.emplace_back(std::move(Out));
		}
	}

	return OutMeshes;
}
