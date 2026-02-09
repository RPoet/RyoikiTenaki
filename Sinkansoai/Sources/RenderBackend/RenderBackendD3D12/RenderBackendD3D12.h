#pragma once

#include "../../Singleton.h"
#include "../RenderBackend.h"
#include "RenderBackendD3D12Common.h"
#include "GraphicsCommandListD3D12.h"
#include "ComputeCommandListD3D12.h"
#include "CopyCommandListD3D12.h"
#include "DynamicBufferD3D12.h"
#include "ResourceBufferD3D12.h"
#include "../../Engine/Mesh.h"

#include "TextureD3D12.h"

#include "../../Math/SIMDMath.h"
// using namespace SIMDMath; // Optional, or use qualified names.
// Vector.h aliases float3/float4 to SIMDMath types.

constexpr static int32 NumBackBuffers = 2;

struct RSceneTextures;

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
	RGraphicsCommandListD3D12 GraphicsCommandList;
	RComputeCommandListD3D12 ComputeCommandList;
	RCopyCommandListD3D12 CopyCommandList;
	TRefCountPtr<ID3D12CommandQueue> CommandQueue;

	vector<RMesh> RenderMeshes;
	RMesh LightVolumeMesh;
	SharedPtr<RTextureD3D12> DefaultTexture;
	SharedPtr<RTextureD3D12> DefaultBlackTexture;
	SharedPtr<RTextureD3D12> DefaultWhiteTexture;
	SharedPtr<RTextureD3D12> DefaultMetalRoughTexture;
	SharedPtr<RTextureD3D12> BackBufferTextures[NumBackBuffers]{};

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
	D3D12_GPU_DESCRIPTOR_HANDLE AddressCacheForDescriptorHeapStart[ToDescriptorIndex(EDescriptorHeapAddressSpace::Num)]{};
	uint32 NumRegisteredHeaps[ToDescriptorIndex(EDescriptorHeapAddressSpace::Num)]{};
	D3D12_CPU_DESCRIPTOR_HANDLE SceneTextureCPUHandle{};
	D3D12_GPU_DESCRIPTOR_HANDLE SceneTextureGPUHandle{};
	bool bSceneTexturesInitialized = false;

	uint32 RTVDescriptorSize;
	uint32 DSVDescriptorSize;
	uint32 CBVSRVUAVDescriptorSize;


	float LastSubmitTime;
	float CurrentSubmitTime;

public:
	RRenderBackendD3D12();
	virtual ~RRenderBackendD3D12() = default;

	virtual void Init() override;
	virtual void Teardown() override;

	ID3D12Device* GetDevice()
	{
		return Device.Get();
	}

	RGraphicsCommandListD3D12& GetMainGraphicsCommandList()
	{
		return GraphicsCommandList;
	}

	RComputeCommandListD3D12& GetMainComputeCommandList()
	{
		return ComputeCommandList;
	}

	RCopyCommandListD3D12& GetMainCopyCommandList()
	{
		return CopyCommandList;
	}

	virtual vector<RMesh*> GetRenderMesh() override;
	RMesh* GetLightVolumeMesh() override;

	uint32 GetSceneRTVCount() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE GetSceneRTVHandle(uint32 Index) override;
	D3D12_CPU_DESCRIPTOR_HANDLE GetSceneDepthHandle() override;
	D3D12_GPU_DESCRIPTOR_HANDLE GetSceneTextureGPUHandle() override;
	void InitSceneTextures(RSceneTextures& SceneTextures) override;

	ID3D12DescriptorHeap* GetCBVSRVHeap() override;
	D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHandle(EDescriptorHeapAddressSpace AddressSpace) override;

	const D3D12_VIEWPORT* GetViewport() override;
	const D3D12_RECT* GetScissorRect() override;

	RGraphicsPipeline* GetGraphicsPipeline(EGraphicsPipeline Pipeline) override;

	D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRTVHandle() override;
	RTexture* GetBackBufferResource() override;

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

	friend class RGraphicsCommandListD3D12;
	friend class RComputeCommandListD3D12;
	friend class RCopyCommandListD3D12;
};
