#pragma once

#include "../../Singleton.h"
#include "../RenderCommandList.h"
#include "RenderBackendD3D12Common.h"

class RRenderBackendD3D12;

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

	void AllocateCommandLsit(RRenderBackendD3D12& Backend);

	friend class RRenderBackendD3D12;
};
