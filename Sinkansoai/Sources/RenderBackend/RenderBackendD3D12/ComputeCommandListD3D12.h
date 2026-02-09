#pragma once

#include "../RenderCommandList.h"
#include "RenderBackendD3D12Common.h"
#include "GraphicsCommandListD3D12.h"

class RComputeCommandListD3D12 : public RComputeCommandList
{
private:
	BackendCommandListCommon UnderlyingCommandList{ CommandListType::Compute };

public:
	RComputeCommandListD3D12() = default;
	virtual ~RComputeCommandListD3D12();

	void AllocateCommandList(RRenderBackendD3D12& Backend);

	void Reset() override;
	void Close() override;

	void BeginEvent(UINT64 Color, const wchar_t* Name) override;
	void EndEvent() override;

	void SumbitResourceBarriers(uint32 NumBarriers, const ResourceBarrier* Barriers) override final;
	void SetDescriptorHeaps(uint32 NumDescriptorHeaps, ID3D12DescriptorHeap* const* DescriptorHeaps) override;
	void SetComputeRootDescriptorTable(uint32 RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) override;
	void SetComputeRoot32BitConstants(uint32 RootParameterIndex, uint32 Num32BitValuesToSet, const void* SrcData, uint32 DestOffsetIn32BitValues) override;
	void SetComputeRoot32BitConstant(uint32 RootParameterIndex, uint32 SrcData, uint32 DestOffsetIn32BitValues) override;
	void Dispatch(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ) override;

	ID3D12GraphicsCommandList* GetRawCommandList()
	{
		return UnderlyingCommandList();
	}

	ID3D12CommandList* GetRawCommandListBase()
	{
		return reinterpret_cast<ID3D12CommandList*>(UnderlyingCommandList());
	}

	friend class RRenderBackendD3D12;
};
