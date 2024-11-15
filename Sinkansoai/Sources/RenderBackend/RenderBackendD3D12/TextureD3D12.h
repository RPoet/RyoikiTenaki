#pragma once

#include "../../Singleton.h"
#include "RenderBackendD3D12Common.h"
#include "../Texture.h"

class RRenderBackendD3D12;
class RTexture2DD3D12 : public RTexture
{
private:
	RRenderBackendD3D12& Backend;

	ComPtr<ID3D12Resource> UnderlyingResource{};


public:
	RTexture2DD3D12(RRenderBackendD3D12& Backend, const String& Name)
		: RTexture(Name)
		, Backend(Backend)
	{}

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
	{ 
		return UnderlyingResource->GetGPUVirtualAddress();
	}

	friend class RRenderBackendD3D12;
};

