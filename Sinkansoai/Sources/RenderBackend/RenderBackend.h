#pragma once
#include "../Definitions.h"
#include "RenderCommandList.h"
#include "DynamicBuffer.h"

struct RMesh;
class RGraphicsPipeline;

enum EResourceType
{
	RenderBuffer = 1,
	RenderTexture1D = 2,
	RenderTexture2D = 3,
	RenderTexture3D = 4,
	Texture2D = 5
};

enum EResourceFlag
{
	None = 0,
	RenderTarget = 1,
	DepthStencilTarget = 2,
	UnorderedAccess = 3,
	DenyShaderResource = 4,
	AllowSimultanenousAccess = 5,
};

enum class ERenderBackendType : uint8
{
	D3D12,
	Vulkan,
	SIMD,
	Compute
};

enum class EGraphicsPipeline : uint8
{
	Prepass,
	Basepass,
	ForwardLighting,
	DeferredLighting,
	DeferredLocalLighting,
	Postprocess,
	NumPasses
};

constexpr uint32 ToPipelineIndex(EGraphicsPipeline Value)
{
	return static_cast<uint32>(Value);
}

enum class EDescriptorHeapAddressSpace : uint8
{
	ConstantBufferView = 0,
	ShaderResourceView = 1,
	UnorderedAccessView = 2,
	Num
};

constexpr uint32 ToDescriptorIndex(EDescriptorHeapAddressSpace Value)
{
	return static_cast<uint32>(Value);
}

inline D3D12_RESOURCE_BARRIER MakeTransitionBarrier(ID3D12Resource* Resource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After)
{
	D3D12_RESOURCE_BARRIER Barrier{};
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource = Resource;
	Barrier.Transition.StateBefore = Before;
	Barrier.Transition.StateAfter = After;
	Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	return Barrier;
}

enum class ESceneTexture : uint8
{
	SceneDepth = 0,
	SceneColor,
	BaseColor,
	WorldNormal,
	Material,
	DebugTexture
};

ERenderBackendType BackendTypeFromName(const String& BackendName);
const String& BackendNameFromType(ERenderBackendType BackendType);


class RRenderBackend
{
protected:
	String BackendName = TEXT("NONE");
	RGraphicsCommandList* MainGraphicsCommandList{};
	RComputeCommandList* MainComputeCommandList{};
	RCopyCommandList* MainCopyCommandList{};

public:

	RRenderBackend() = default;
	virtual ~RRenderBackend() = default;

	virtual void Init() {};
	virtual void Teardown() {};
	virtual void RenderBegin() {};
	virtual void RenderFinish() {};

	void SetBackendName(const String& NAME)
	{
		BackendName = NAME;
	}

	const String& GetBackendName()
	{
		return BackendName;
	}

	RGraphicsCommandList* GetMainGraphicsCommandList()
	{
		return MainGraphicsCommandList;
	}

	RComputeCommandList* GetMainComputeCommandList()
	{
		return MainComputeCommandList;
	}

	RCopyCommandList* GetMainCopyCommandList()
	{
		return MainCopyCommandList;
	}

	virtual RDynamicBuffer* GetGlobalDynamicBuffer(uint32 Index) { return nullptr;  }

	virtual RMesh* GetRenderMesh() { return nullptr; }
	virtual RMesh* GetLightVolumeMesh() { return nullptr; }

	virtual uint32 GetSceneRTVCount() const { return 0; }
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetSceneRTVHandle(uint32 Index) { return {}; }
	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetSceneDepthHandle() { return {}; }
	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetSceneTextureGPUHandle() { return {}; }

	virtual ID3D12DescriptorHeap* GetCBVSRVHeap() { return nullptr; }
	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHandle(EDescriptorHeapAddressSpace) { return {}; }

	virtual const D3D12_VIEWPORT* GetViewport() { return nullptr; }
	virtual const D3D12_RECT* GetScissorRect() { return nullptr; }

	virtual RGraphicsPipeline* GetGraphicsPipeline(EGraphicsPipeline) { return nullptr; }
	virtual ID3D12Resource* GetSceneTextureResource(ESceneTexture) { return nullptr; }

	virtual D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRTVHandle() { return {}; }
	virtual ID3D12Resource* GetBackBufferResource() { return nullptr; }
};

