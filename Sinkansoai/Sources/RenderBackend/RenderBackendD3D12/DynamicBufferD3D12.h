#pragma once

#include "../../Singleton.h"
#include "RenderBackendD3D12Common.h"
#include "../DynamicBuffer.h"

class RRenderBackendD3D12;
class RDynamicBufferD3D12 : public RDynamicBuffer
{
private:
	RRenderBackendD3D12& Backend;

	ComPtr<ID3D12Resource> UnderlyingResource{};


public:
	RDynamicBufferD3D12(RRenderBackendD3D12& Backend, const String& Name)
		: RDynamicBuffer(Name)
		, Backend(Backend)
	{}


	void Allocate(const uint32 Size);

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
	{ 
		return UnderlyingResource->GetGPUVirtualAddress();
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC GetCBVDesc()
	{
		assert(Size > 0u);
		D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc = {};
		CBVDesc.BufferLocation = GetGPUVirtualAddress();
		CBVDesc.SizeInBytes = Size;

		return CBVDesc;
	}

	friend class RRenderBackendD3D12;
};

