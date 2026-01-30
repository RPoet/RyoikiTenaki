#pragma once

#include "../RenderCommandList.h"
#include "RenderBackendD3D12Common.h"

class RRenderBackendD3D12;

class RCopyCommandListD3D12 : public RCopyCommandList
{
private:
	ID3D12CommandAllocator* CommandAllocator{};
	ID3D12GraphicsCommandList* CommandList{};

public:
	RCopyCommandListD3D12() = default;
	virtual ~RCopyCommandListD3D12();

	void AllocateCommandList(RRenderBackendD3D12& Backend);

	void Reset() override;
	void Close() override;

	void ResourceBarrier(uint32 NumBarriers, const D3D12_RESOURCE_BARRIER* Barriers) override;
	void CopyBufferRegion(ID3D12Resource* Dest, UINT64 DestOffset, ID3D12Resource* Src, UINT64 SrcOffset, UINT64 NumBytes) override;
	void CopyTexture(void* pData, ID3D12Resource* Dest, ID3D12Resource* UploadHeap, uint32 TextureWidth, uint32 Height, uint32 PixelSizeInBytes) override;

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
