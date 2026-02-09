#pragma once

#include "../RenderCommandList.h"
#include "RenderBackendD3D12Common.h"
#include "MemoryScratchPad.h"

class RRenderBackendD3D12;
struct ResourceBarrier;

class BackendCommandListCommon : public BackendCommandList
{
protected:
	ID3D12CommandAllocator* CommandAllocator{};
	ID3D12GraphicsCommandList* CommandList{};
	BackendCommandListMemoryScratchPad ScratchPad;

	CommandListType Type = NumCommandList;
	uint32 Counter = 0;

public:
	BackendCommandListCommon(CommandListType Type)
		: Type(Type)
	{
	};

	virtual ~BackendCommandListCommon();
	void AllocateCommandList(RRenderBackendD3D12& Backend, D3D12_COMMAND_LIST_TYPE CommandListType);
	void BeginEvent(UINT64 Color, const wchar_t* Name)  override final;
	void EndEvent() override final;
	void Reset() override final;
	void Close() override final;
	void SumbitResourceBarriers(uint32 NumBarriers, const ResourceBarrier* Barriers) override final;

	inline ID3D12GraphicsCommandList* operator()()
	{
		return CommandList;
	}

	friend class RGraphicsCommandListD3D12;
	friend class RComputeCommandListD3D12;
	friend class RCopyCommandListD3D12;
};

class RGraphicsCommandListD3D12 : public RGraphicsCommandList
{
private:
	BackendCommandListCommon UnderlyingCommandList{ CommandListType::Graphics };

public:
	RGraphicsCommandListD3D12() = default;
	virtual ~RGraphicsCommandListD3D12();

	void AllocateCommandList(RRenderBackendD3D12& Backend);

	void Reset() override;
	void Close() override;
	void BeginEvent(UINT64 Color, const wchar_t* Name) override;
	void EndEvent() override;

	void SumbitResourceBarriers(uint32 NumBarriers, const ResourceBarrier* Barriers) override final;
	void SetViewports(uint32 NumViewports, const D3D12_VIEWPORT* Viewports) override;
	void SetScissorRects(uint32 NumRects, const D3D12_RECT* Rects) override;

	void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const float Color[4], uint32 NumRects, const D3D12_RECT* Rects) override;
	void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags, float Depth, uint8 Stencil, uint32 NumRects, const D3D12_RECT* Rects) override;
	void OMSetRenderTargets(uint32 NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* RenderTargetDescriptors, bool bSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* DepthStencilDescriptor) override;

	void SetDescriptorHeaps(uint32 NumDescriptorHeaps, ID3D12DescriptorHeap* const* DescriptorHeaps) override;
	void SetGraphicsRootDescriptorTable(uint32 RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) override;
	void SetGraphicsRoot32BitConstants(uint32 RootParameterIndex, uint32 Num32BitValuesToSet, const void* SrcData, uint32 DestOffsetIn32BitValues) override;
	void SetGraphicsRoot32BitConstant(uint32 RootParameterIndex, uint32 SrcData, uint32 DestOffsetIn32BitValues) override;

	void SetGraphicsPipeline(RGraphicsPipeline& Pipeline) override;
	void SetPrimitiveTopology(PrimitiveType Type) override;
	void SetVertexBuffer(uint32 Slot, RVertexBuffer* Buffer) override;
	void SetIndexBuffer(RIndexBuffer* Buffer) override;

	void DrawIndexedInstanced(uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, int32 BaseVertexLocation, uint32 StartInstanceLocation) override;
	void DrawInstanced(uint32 VertexCountPerInstance, uint32 InstanceCount, uint32 StartVertexLocation, uint32 StartInstanceLocation) override;

	void CopyTexture(void* pData, ID3D12Resource* Dest, ID3D12Resource* UploadHeap, uint32 TextureWidth, uint32 Height, uint32 PixelSizeInBytes) override;

	inline ID3D12GraphicsCommandList* GetRawCommandList()
	{
		return UnderlyingCommandList();
	}

	inline ID3D12CommandList* GetRawCommandListBase()
	{
		return reinterpret_cast<ID3D12CommandList*>(UnderlyingCommandList());
	}

	friend class RRenderBackendD3D12;
};
