#pragma once
#include "Definitions.h"
#include "SceneGraphSystem/GraphEntity.h"

enum class EObjectNodeType : uint8
{
	Root,
	Camera,
	Light,
	Entity
};

struct MObjectNode
{
	String Name;
	EObjectNodeType Type = EObjectNodeType::Entity;
	MGraphEntity* Entity = nullptr;
	int32 Parent = -1;
	vector<int32> Children;
};

struct MObjectGraph
{
	vector<MObjectNode> Nodes;
	int32 RootIndex = -1;

	void Reset()
	{
		Nodes.clear();
		RootIndex = -1;
	}

	int32 AddNode(const String& Name, EObjectNodeType Type, MGraphEntity* Entity, int32 ParentIndex)
	{
		const int32 Index = static_cast<int32>(Nodes.size());
		MObjectNode Node{};
		Node.Name = Name;
		Node.Type = Type;
		Node.Entity = Entity;
		Node.Parent = ParentIndex;
		Nodes.push_back(Node);

		if (ParentIndex >= 0 && ParentIndex < static_cast<int32>(Nodes.size()))
		{
			Nodes[ParentIndex].Children.push_back(Index);
		}
		return Index;
	}

	MObjectNode* GetNode(int32 Index)
	{
		if (Index < 0 || Index >= static_cast<int32>(Nodes.size()))
		{
			return nullptr;
		}
		return &Nodes[Index];
	}

	const MObjectNode* GetNode(int32 Index) const
	{
		if (Index < 0 || Index >= static_cast<int32>(Nodes.size()))
		{
			return nullptr;
		}
		return &Nodes[Index];
	}
};
