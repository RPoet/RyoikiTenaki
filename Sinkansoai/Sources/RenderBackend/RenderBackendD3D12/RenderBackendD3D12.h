#pragma once

#include "../../Singleton.h"
#include "../RenderBackend.h"
#include "RenderBackendD3D12Common.h"
#include "RenderCommandListD3D12.h"
#include "DynamicBufferD3D12.h"
#include "ResourceBufferD3D12.h"

/// <summary>
/// TO DO : Remove DirectXMath here
/// </summary>
#include <DirectXMath.h>
using namespace DirectX;

constexpr static int32 NumBackBuffers = 2;

class RRenderBackendD3D12 : public RRenderBackend, public Singleton<RRenderBackendD3D12>
{
private:
	TRefCountPtr< ID3D12Device > Device;
	vector<RRenderCommandListD3D12> CommandLists;
	TRefCountPtr<ID3D12CommandQueue> CommandQueue;


	// Should be moved and managed separated class
	TRefCountPtr<ID3D12RootSignature> RootSignature;
	TRefCountPtr<ID3D12PipelineState> PipelineStateObject;


	// Make Heap manager. Scene ConstatnBuffers
	TRefCountPtr<ID3D12DescriptorHeap> CBVHeap;

	// Make Heap manager. backbuffer RTV Heap
	TRefCountPtr<ID3D12DescriptorHeap> RTVHeap;

	// Make Heap manager. backbuffer DSV Heap
	TRefCountPtr<ID3D12DescriptorHeap> DSVHeap;

	uint32 RTVDescriptorSize;
	uint32 DSVDescriptorSize;
	uint32 CBVSRVUAVDescriptorSize;

	RVertexBufferD3D12 PositionVertexBuffer;
	RVertexBufferD3D12 ColorVertexBuffer;
	RVertexBufferD3D12 UVVertexBuffer;

	RIndexBufferD3D12 IndexBuffer;

	// App resources.
	//TRefCountPtr<ID3D12Resource> VertexBuffer;
	//D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

	// Synchronization objects.
	uint32 FrameIndex;
	HANDLE FenceEvent;
	ComPtr<ID3D12Fence> Fence;
	UINT64 FenceValue;


	CD3DX12_VIEWPORT Viewport;
	CD3DX12_RECT ScissorRect;
	TRefCountPtr<IDXGISwapChain3> SwapChain;
	TRefCountPtr<ID3D12Resource> RenderTargets[NumBackBuffers];
	TRefCountPtr<ID3D12Resource> DepthStencilBuffer;
	
	RDynamicBufferD3D12 DynamicBuffer;

public:
	RRenderBackendD3D12();
	virtual ~RRenderBackendD3D12() = default;

	virtual void Init() override;
	virtual void Teardown() override;
	virtual void FunctionalityTestRender() override;

	ID3D12Device* GetDevice()
	{
		return Device.Get();
	}

	void WaitForPreviousFence();


	virtual RDynamicBuffer* GetGlobalDynamicBuffer() override
	{
		return &DynamicBuffer;
	}

	TRefCountPtr<ID3D12Resource> CreateUnderlyingResource(EResourceType ResourceType, EResourceFlag ResourceFlag, DXGI_FORMAT Format, uint32 Width, uint32 Height, uint32 NumMips, uint32 Depths);


	TRefCountPtr<ID3D12Resource> CreateTexture2DResource(String&& Name, EResourceFlag ResourceFlag, DXGI_FORMAT Format, uint32 Width, uint32 Height);

	friend class RRenderCommandListD3D12;
};
