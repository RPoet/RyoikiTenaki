#pragma once

#include "../../Singleton.h"
#include "../RenderBackend.h"
#include "RenderBackendD3D12Common.h"
#include "RenderCommandListD3D12.h"
#include "DynamicBufferD3D12.h"


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

	uint32 RTVDescriptorSize;
	uint32 CBVSRVUAVDescriptorSize;

	// App resources.
	TRefCountPtr<ID3D12Resource> VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;

	// Synchronization objects.
	uint32 FrameIndex;
	HANDLE FenceEvent;
	ComPtr<ID3D12Fence> Fence;
	UINT64 FenceValue;

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	CD3DX12_VIEWPORT Viewport;
	CD3DX12_RECT ScissorRect;
	TRefCountPtr<IDXGISwapChain3> SwapChain;
	TRefCountPtr<ID3D12Resource> RenderTargets[NumBackBuffers];


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

	friend class RRenderCommandListD3D12;
};
