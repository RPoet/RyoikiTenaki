#pragma once
#include "../PlatformDefinitions.h"

class RRenderResource;

enum class ResourceState : uint32
{
	Common = 0,
	RenderTarget,
	DepthWrite,
	DepthRead,
	ShaderResource,
	CopyDest,
	CopySource,
	Present,
	GenericRead,
	UnorderedAccess
};

enum class ResourceBarrierType : uint8
{
	Transition = 0,
	Aliasing = 1,
	UAV = 2,
};

enum class ResourceBarrierFlag : uint8
{
	None = 0,
	BeginOnly = 0x1,
	EndOnly = 0x2
};

constexpr uint32 ResourceBarrierAllSubresources = 0xffffffffu;

struct ResourceTransitionBarrier
{
	RRenderResource* Resource = nullptr;
	uint32 Subresource = ResourceBarrierAllSubresources;
	ResourceState StateBefore = ResourceState::Common;
	ResourceState StateAfter = ResourceState::Common;
};

struct ResourceAliasingBarrier
{
	RRenderResource* ResourceBefore = nullptr;
	RRenderResource* ResourceAfter = nullptr;
};

struct ResourceUAVBarrier
{
	RRenderResource* Resource = nullptr;
};

struct ResourceBarrier
{
	ResourceBarrierType Type = ResourceBarrierType::Transition;
	ResourceBarrierFlag Flags = ResourceBarrierFlag::None;
	union
	{
		ResourceTransitionBarrier Transition;
		ResourceAliasingBarrier Aliasing;
		ResourceUAVBarrier UAV;
	};
};

inline ResourceBarrier MakeTransitionBarrier(RRenderResource* Resource, ResourceState Before, ResourceState After, uint32 Subresource = ResourceBarrierAllSubresources)
{
	ResourceBarrier Barrier{};
	Barrier.Type = ResourceBarrierType::Transition;
	Barrier.Flags = ResourceBarrierFlag::None;
	Barrier.Transition.Resource = Resource;
	Barrier.Transition.StateBefore = Before;
	Barrier.Transition.StateAfter = After;
	Barrier.Transition.Subresource = Subresource;
	return Barrier;
}
