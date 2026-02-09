#pragma once

#include "../RenderCommandList.h"
#include "RenderBackendD3D12Common.h"
#include "GraphicsCommandListD3D12.h"

class RCopyCommandListD3D12 : public RCopyCommandList
{
private:
	BackendCommandListCommon UnderlyingCommandList{ CommandListType::Copy };

public:
	RCopyCommandListD3D12() = default;
	virtual ~RCopyCommandListD3D12();

	void AllocateCommandList(RRenderBackendD3D12& Backend);

	void Reset() override;
	void Close() override;

	void BeginEvent(UINT64 Color, const wchar_t* Name) override;
	void EndEvent() override;

	void SumbitResourceBarriers(uint32 NumBarriers, const ResourceBarrier* Barriers) override final;
	void CopyBufferRegion(ID3D12Resource* Dest, UINT64 DestOffset, ID3D12Resource* Src, UINT64 SrcOffset, UINT64 NumBytes) override;
	void CopyTexture(void* pData, ID3D12Resource* Dest, ID3D12Resource* UploadHeap, uint32 TextureWidth, uint32 Height, uint32 PixelSizeInBytes) override;

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
