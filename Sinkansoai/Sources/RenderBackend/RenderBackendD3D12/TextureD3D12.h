#pragma once
#include "../Texture.h"

#include "RenderBackendD3D12Common.h"


class RRenderBackendD3D12;
class RTexture2DD3D12 : public RTexture
{
private:
	RRenderBackendD3D12& Backend;

	TRefCountPtr<ID3D12Resource> UnderlyingResource{};
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc;

	DXGI_FORMAT Format;
	EResourceFlag Flag;
public:

	RTexture2DD3D12(RRenderBackendD3D12& Backend, const String& Name, uint32 Width, uint32 Height, uint32 NumMips, DXGI_FORMAT Format, EResourceFlag Flag = EResourceFlag::None)
		: RTexture(Name)
		, Backend(Backend)
		, UnderlyingResource{}
		, SRVDesc{}
		, Format(Format)
		, Flag(Flag)
	{
		this->Width = Width;
		this->Height = Height;
		this->NumMips = NumMips;

		this->PixelSizeInBytes = GetPerFormatPixelSizeInBytes(Format);
	}

	virtual void AllocateResource() override;

	virtual void StreamTexture(void* pData) override;

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
	{ 
		return UnderlyingResource->GetGPUVirtualAddress();
	}

	const D3D12_SHADER_RESOURCE_VIEW_DESC& GetSRVDesc() const
	{
		return SRVDesc;
	}

	ID3D12Resource* GetUnderlyingResource() const
	{
		return UnderlyingResource.Get();
	}


	friend class RRenderBackendD3D12;
};

