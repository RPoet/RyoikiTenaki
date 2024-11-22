#pragma once
#include "../Texture.h"
#include "RenderBackendD3D12Common.h"

class RRenderBackendD3D12;

class RUnderlyingResourceD3D12
{
protected:
	TRefCountPtr<ID3D12Resource> UnderlyingResource{};


public:
	RUnderlyingResourceD3D12() = default;
	virtual ~RUnderlyingResourceD3D12() = default;

	ID3D12Resource* GetUnderlyingResource() const
	{
		return UnderlyingResource.Get();
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
	{
		return UnderlyingResource->GetGPUVirtualAddress();
	}
};


class RTextureD3D12 : public RTexture, public RUnderlyingResourceD3D12
{
protected:
	RRenderBackendD3D12& Backend;
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	DXGI_FORMAT Format;
	EResourceFlag Flag;

public:

	RTextureD3D12(RRenderBackendD3D12& Backend, const String& Name, uint32 Width, uint32 Height, uint32 NumMips, DXGI_FORMAT Format, EResourceFlag Flag = EResourceFlag::None)
		: RTexture(Name)
		, RUnderlyingResourceD3D12()
		, Backend(Backend)
		, SRVDesc{}
		, Format(Format)
		, Flag(Flag)
	{
		this->Width = Width;
		this->Height = Height;
		this->NumMips = NumMips;
		this->PixelSizeInBytes = GetPerFormatPixelSizeInBytes(Format);
	}

	const D3D12_SHADER_RESOURCE_VIEW_DESC& GetSRVDesc() const
	{
		return SRVDesc;
	}


	virtual void SetSRVFormat(DXGI_FORMAT Format)
	{
		SRVDesc.Format = Format;
	}
	friend class RRenderBackendD3D12;
};

class RTexture2DD3D12 : public RTextureD3D12
{
private:

public:
	RTexture2DD3D12(RRenderBackendD3D12& Backend, const String& Name, uint32 Width, uint32 Height, uint32 NumMips, DXGI_FORMAT Format, EResourceFlag Flag = EResourceFlag::None)
		: RTextureD3D12(Backend, Name, Width, Height, NumMips, Format, Flag)
	{}

	virtual void AllocateResource() override;

	virtual void StreamTexture(void* pData) override;

	friend class RRenderBackendD3D12;
};

class RRenderTargetD3D12 : public RTextureD3D12
{
private:

public:

	RRenderTargetD3D12(RRenderBackendD3D12& Backend, const String& Name, uint32 Width, uint32 Height, uint32 NumMips, DXGI_FORMAT Format, EResourceFlag Flag = EResourceFlag::None)
		: RTextureD3D12(Backend, Name, Width, Height, NumMips, Format, Flag)
	{}

	virtual void AllocateResource() override;

};
