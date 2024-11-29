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
	DeferredLighting,
	Postprocess,
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
	SharedPtr< RRenderTargetD3D12 > DebugTexture;

	D3D12_GPU_DESCRIPTOR_HANDLE GPUAddressHandle{};


	void InitSceneTextures(RRenderBackendD3D12& Backend);
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


// TO DO implement Descriptor heap cache
//class RDescriptorHeap
//{
//private:
//	TRefCountPtr<ID3D12DescriptorHeap> UnderlyingHeap;
//
//public:
//
//
//	ID3D12DescriptorHeap* GetUnderlyingHeap()
//	{
//		return UnderlyingHeap.Get();
//	}
//};

using RDescriptorHeap = ID3D12DescriptorHeap;

class RRenderBackendD3D12 : public RRenderBackend, public Singleton<RRenderBackendD3D12>
{
private:
	uint32 bSupportsRootSignatureVersion1_1 : 1;

	TRefCountPtr< ID3D12Device > Device;
	vector<RRenderCommandListD3D12> CommandLists;
	TRefCountPtr<ID3D12CommandQueue> CommandQueue;

	vector< RGraphicsPipeline > GraphicsPipelines;
	// Synchronization objects.
	uint32 FrameIndex;
	HANDLE FenceEvent;
	ComPtr<ID3D12Fence> Fence;
	UINT64 FenceValue;

	CD3DX12_VIEWPORT Viewport;
	CD3DX12_RECT ScissorRect;
	TRefCountPtr<IDXGISwapChain3> SwapChain;
	TRefCountPtr<ID3D12Resource> RenderTargets[NumBackBuffers];

	RDynamicBufferD3D12 DynamicBuffers[2];

	vector< TRefCountPtr<ID3D12Resource> > UploadHeapReferences;


	// Make Heap manager. Scene ConstatnBuffers

	TRefCountPtr<RDescriptorHeap> SceneTextureRTVHeap;
	TRefCountPtr<RDescriptorHeap> ScreenPassRTVHeap;

	TRefCountPtr<RDescriptorHeap> RTVHeap;
	TRefCountPtr<RDescriptorHeap> DSVHeap;
	TRefCountPtr<RDescriptorHeap> CBVSRVHeap;
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
	void RenderLights(RRenderCommandListD3D12& CommandList);
	void Postprocess();

	virtual void FunctionalityTestRender() override;

	ID3D12Device* GetDevice()
	{
		return Device.Get();
	}

	RRenderCommandListD3D12& GetMainCommandList()
	{
		return CommandLists[0];
	}

	void Execute();
	void WaitForPreviousFence();

	void RenderBegin() override final;
	void RenderFinish() override final;


	virtual RDynamicBuffer* GetGlobalDynamicBuffer(uint32 Index) override
	{
		assert(Index < 2 && " Increase Fixed size dyanmic buffer");
		return &DynamicBuffers[Index];
	}


	TRefCountPtr<ID3D12Resource> CreateUnderlyingResource(EResourceType ResourceType, EResourceFlag ResourceFlag, DXGI_FORMAT Format, uint32 Width, uint32 Height, uint32 NumMips, uint32 Depths);

	TRefCountPtr<ID3D12Resource> CreateRenderTargetResource(String&& Name, EResourceFlag ResourceFlag, DXGI_FORMAT Format, uint32 Width, uint32 Height);
	TRefCountPtr<ID3D12Resource> CreateTexture2DResource(String&& Name, EResourceFlag ResourceFlag, DXGI_FORMAT Format, uint32 Width, uint32 Height);
	TRefCountPtr<ID3D12Resource> CreateUploadHeap(const uint32 UploadHeapSize);



	void CreateRootSignature(const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& RootSignatureDesc, RGraphicsPipeline& Pipepline);

	friend class RRenderCommandListD3D12;
};
