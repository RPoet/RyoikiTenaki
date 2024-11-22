#pragma once

#include "../../Singleton.h"
#include "../RenderBackend.h"
#include "RenderBackendD3D12Common.h"
#include "RenderCommandListD3D12.h"
#include "DynamicBufferD3D12.h"
#include "ResourceBufferD3D12.h"

#include "TextureD3D12.h"

/// <summary>
/// TO DO : Remove DirectXMath here
/// </summary>
#include <DirectXMath.h>
using namespace DirectX;

constexpr static int32 NumBackBuffers = 2;


enum EGraphicsPipeline
{
	Prepass,
	Basepass,
	DeferredLighting ,
	NumPasses = DeferredLighting + 1
};

enum EDescriptorHeapAddressSpace
{
	ConstantBufferView  = 0,
	ShaderResourceView  = 1,
	UnorderedAccessView = 2,

	Num = UnorderedAccessView + 1
};

struct RSceneTextures
{
	SharedPtr< RRenderTargetD3D12 > SceneDepth;
	SharedPtr< RRenderTargetD3D12 > SceneColor;
	SharedPtr< RRenderTargetD3D12 > BaseColor;
	SharedPtr< RRenderTargetD3D12 > WorldNormal;
	SharedPtr< RRenderTargetD3D12 > Material;


	D3D12_GPU_DESCRIPTOR_HANDLE GPUAddressHandle{};
};

class RGraphicsPipeline
{
private:

	TRefCountPtr<ID3D12RootSignature> RootSignature;

	TRefCountPtr<ID3D12PipelineState> PipelineStateObject;

public:


	void SetRootSignature(ID3D12RootSignature* RootSignature)
	{
		this->RootSignature = RootSignature;
	}

	void SetPSO(ID3D12PipelineState* PipelineStateObject)
	{
		this->PipelineStateObject = PipelineStateObject;
	}

	TRefCountPtr<ID3D12RootSignature>& GetRootSignature()
	{
		return RootSignature;
	}

	TRefCountPtr<ID3D12PipelineState>& GetPipelineStateObject()
	{
		return PipelineStateObject;
	}
};


class RRenderBackendD3D12 : public RRenderBackend, public Singleton<RRenderBackendD3D12>
{
private:
	TRefCountPtr< ID3D12Device > Device;
	vector<RRenderCommandListD3D12> CommandLists;
	TRefCountPtr<ID3D12CommandQueue> CommandQueue;

	vector< RGraphicsPipeline > GraphicsPipelines;

	//TRefCountPtr<ID3D12RootSignature> RootSignature;
	//TRefCountPtr<ID3D12PipelineState> PipelineStateObject;

	// Synchronization objects.
	uint32 FrameIndex;
	HANDLE FenceEvent;
	ComPtr<ID3D12Fence> Fence;
	UINT64 FenceValue;

	CD3DX12_VIEWPORT Viewport;
	CD3DX12_RECT ScissorRect;
	TRefCountPtr<IDXGISwapChain3> SwapChain;
	TRefCountPtr<ID3D12Resource> RenderTargets[NumBackBuffers];


	RDynamicBufferD3D12 DynamicBuffer;

	vector< TRefCountPtr<ID3D12Resource> > UploadHeapReferences;


	// Make Heap manager. Scene ConstatnBuffers

	TRefCountPtr<ID3D12DescriptorHeap> SceneTextureRTVHeap;

	TRefCountPtr<ID3D12DescriptorHeap> RTVHeap;
	TRefCountPtr<ID3D12DescriptorHeap> DSVHeap;
	TRefCountPtr<ID3D12DescriptorHeap> CBVSRVHeap;
	D3D12_GPU_DESCRIPTOR_HANDLE AddressCacheForDescriptorHeapStart[EDescriptorHeapAddressSpace::Num]{};
	uint32 NumRegisteredHeaps[EDescriptorHeapAddressSpace::Num]{};

	uint32 RTVDescriptorSize;
	uint32 DSVDescriptorSize;
	uint32 CBVSRVUAVDescriptorSize;


public:
	RRenderBackendD3D12();
	virtual ~RRenderBackendD3D12() = default;

	virtual void Init() override;
	virtual void Teardown() override;

	void Prepass();
	void Basepass();

	void AfterBasepass();

	virtual void FunctionalityTestRender() override;

	ID3D12Device* GetDevice()
	{
		return Device.Get();
	}

	RRenderCommandListD3D12& GetMainCommandList()
	{
		return CommandLists[0];
	}

	void WaitForPreviousFence();

	void RenderFinish();


	virtual RDynamicBuffer* GetGlobalDynamicBuffer() override
	{
		return &DynamicBuffer;
	}

	TRefCountPtr<ID3D12Resource> CreateUnderlyingResource(EResourceType ResourceType, EResourceFlag ResourceFlag, DXGI_FORMAT Format, uint32 Width, uint32 Height, uint32 NumMips, uint32 Depths);


	TRefCountPtr<ID3D12Resource> CreateRenderTargetResource(String&& Name, EResourceFlag ResourceFlag, DXGI_FORMAT Format, uint32 Width, uint32 Height);
	TRefCountPtr<ID3D12Resource> CreateTexture2DResource(String&& Name, EResourceFlag ResourceFlag, DXGI_FORMAT Format, uint32 Width, uint32 Height);


	TRefCountPtr<ID3D12Resource> CreateUploadHeap(const uint32 UploadHeapSize);


	friend class RRenderCommandListD3D12;
};
