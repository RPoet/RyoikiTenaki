#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>


#include "MeshBuilder.h"
#include "../Engine.h"


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


IMPLEMENT_MODULE(MMeshBuilder)

void MMeshBuilder::Init()
{
	Super::Init();
	
	cout << "Mesh Builder Init" << endl;

}


void MMeshBuilder::Teardown()
{
	Super::Teardown();
}

MMesh MMeshBuilder::LoadMesh(const String& Path)
{
	MMesh Out{};

	vector<float3> positions;
	vector<float3> normals;
	vector<float2> texCoords;

	unordered_map<Vertex, uint32_t> uniqueVertices = {};

	std::ifstream file(Path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << endl;
		return Out;
	}

	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream ss(line);
		std::string prefix;
		ss >> prefix;

		if (prefix == "v")
		{
			float3 position;
			ss >> position.x >> position.y >> position.z;
			positions.push_back(position);
		}
		else if (prefix == "vn") 
		{
			float3 normal;
			ss >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (prefix == "vt") 
		{
			float2 texCoord;
			ss >> texCoord.x >> texCoord.y;
			texCoords.push_back(texCoord);
		}
		else if(prefix == "f")
		{   // 면(Face)
			std::string vertexData;
			Vertex vertex;
			std::vector<uint32_t> faceIndices; // 임시로 면 인덱스를 저장할 벡터

			for (int i = 0; i < 3; i++) { // 삼각형 면 기준
				ss >> vertexData;

				// 각 인덱스 파싱 (v/vt/vn 형식)
				size_t pos1 = vertexData.find('/');
				size_t pos2 = vertexData.find('/', pos1 + 1);

				int posIndex = std::stoi(vertexData.substr(0, pos1)) - 1;
				int texIndex = std::stoi(vertexData.substr(pos1 + 1, pos2 - pos1 - 1)) - 1;
				int normIndex = std::stoi(vertexData.substr(pos2 + 1)) - 1;

				vertex.position = positions[posIndex];
				vertex.texCoord = texCoords[texIndex];
				vertex.normal = normals[normIndex];

				// 중복된 정점인지 확인
				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(Out.RenderData.Positions.size());
					Out.RenderData.Positions.push_back(vertex.position);
					Out.RenderData.UV0.push_back(vertex.texCoord);
					Out.RenderData.Normals.push_back(vertex.normal);
				}
				faceIndices.push_back(uniqueVertices[vertex]);
			}

			// DirectX 시계 방향(CW)에 맞추기 위해 인덱스 순서를 반대로 추가
			Out.RenderData.Indices.push_back(faceIndices[2]);
			Out.RenderData.Indices.push_back(faceIndices[1]);
			Out.RenderData.Indices.push_back(faceIndices[0]);
		}
	}

	file.close();

	return Out;
}
