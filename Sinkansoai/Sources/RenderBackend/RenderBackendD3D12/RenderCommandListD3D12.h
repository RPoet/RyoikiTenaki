#pragma once

#include "../../Singleton.h"
#include "../RenderCommandList.h"
#include "RenderBackendD3D12Common.h"

class RRenderBackendD3D12;

using PrimitiveType = uint32;

// TO DO Need to separate compute command list with graphics commnad list
class RRenderCommandListD3D12 : public RRenderCommandList
{
private:

	ID3D12CommandAllocator* CommandAllocator{};
	ID3D12GraphicsCommandList* CommandList{};

	bool bClose = false;

public:
	RRenderCommandListD3D12() = default;
	virtual ~RRenderCommandListD3D12();

	void Reset();
	void Close();

	void AllocateCommandLsit(RRenderBackendD3D12& Backend);

	void CopyTexture(void* pData, ID3D12Resource* Dest, ID3D12Resource* UploadHeap, const uint32 TextureWidth, const uint32 Height, const uint32 PixelSizeInBytes);


	void SetGraphicsPipeline(RGraphicsPipeline& Pipeline);
	void SetPrimitiveTopology(PrimitiveType Type);
	void SetVertexBuffer(uint32 Slot, RVertexBuffer* Buffer);
	void SetIndexBuffer(RIndexBuffer* Buffer);


	ID3D12GraphicsCommandList* GetRawCommandList()
	{
		return CommandList;
	}

	friend class RRenderBackendD3D12;
};
