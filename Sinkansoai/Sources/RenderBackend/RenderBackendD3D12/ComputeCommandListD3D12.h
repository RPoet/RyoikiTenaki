#pragma once

#include "../RenderCommandList.h"
#include "RenderBackendD3D12Common.h"

class RRenderBackendD3D12;

class RComputeCommandListD3D12 : public RComputeCommandList
{
private:
	ID3D12CommandAllocator* CommandAllocator{};
	ID3D12GraphicsCommandList* CommandList{};

public:
	RComputeCommandListD3D12() = default;
	virtual ~RComputeCommandListD3D12();

	void AllocateCommandList(RRenderBackendD3D12& Backend);

	void Reset() override;
	void Close() override;

	void ResourceBarrier(uint32 NumBarriers, const D3D12_RESOURCE_BARRIER* Barriers) override;
	void SetDescriptorHeaps(uint32 NumDescriptorHeaps, ID3D12DescriptorHeap* const* DescriptorHeaps) override;
	void SetComputeRootDescriptorTable(uint32 RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) override;
	void SetComputeRoot32BitConstants(uint32 RootParameterIndex, uint32 Num32BitValuesToSet, const void* SrcData, uint32 DestOffsetIn32BitValues) override;
	void SetComputeRoot32BitConstant(uint32 RootParameterIndex, uint32 SrcData, uint32 DestOffsetIn32BitValues) override;
	void Dispatch(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ) override;

	void BeginEvent(UINT64 Color, const wchar_t* Name) override;
	void EndEvent() override;

	ID3D12GraphicsCommandList* GetRawCommandList()
	{
		return CommandList;
	}

	ID3D12CommandList* GetRawCommandListBase()
	{
		return reinterpret_cast<ID3D12CommandList*>(CommandList);
	}

	friend class RRenderBackendD3D12;
};
