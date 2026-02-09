#pragma once
#include "../Texture.h"
#include "RenderBackendD3D12Common.h"

class RRenderBackendD3D12;

class RTextureD3D12 : public RTexture
{
protected:
	TRefCountPtr<ID3D12Resource> UnderlyingResource{};

	RRenderBackendD3D12& Backend;
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	DXGI_FORMAT Format;
	EResourceFlag Flag;
	EResourceType ResourceType;
	bool bRTVGenerated = false;
	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAddress{};

public:

	RTextureD3D12(RRenderBackendD3D12& Backend, const String& Name, uint32 Width, uint32 Height, uint32 NumMips, DXGI_FORMAT Format, EResourceFlag Flag = EResourceFlag::None, EResourceType ResourceType = EResourceType::Texture2D)
		: RTexture(Name)
		, Backend(Backend)
		, SRVDesc{}
		, Format(Format)
		, Flag(Flag)
		, ResourceType(ResourceType)
	{
		this->Width = Width;
		this->Height = Height;
		this->NumMips = NumMips;
		this->PixelSizeInBytes = GetPerFormatPixelSizeInBytes(Format);

		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.Format = Format;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = NumMips;
	}

	const D3D12_SHADER_RESOURCE_VIEW_DESC& GetSRVDesc() const
	{
		return SRVDesc;
	}

	virtual void SetSRVFormat(DXGI_FORMAT Format)
	{
		SRVDesc.Format = Format;
	}

	virtual void AllocateResource() override;
	virtual void StreamTexture(void* pData) override;

	void SetUnderlyingResource(ID3D12Resource* Resource)
	{
		UnderlyingResource = Resource;
		bInitialized = (Resource != nullptr);
	}

	const void* GetUnderlyingResource() const override final
	{
		return reinterpret_cast<void*>(UnderlyingResource.Get());
	}
	
	void* GetUnderlyingResource() override final
	{
		return reinterpret_cast<void*>(UnderlyingResource.Get());
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
	{
		return UnderlyingResource->GetGPUVirtualAddress();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorAddress() const
	{
		assert(bRTVGenerated && " RTV Should be generated first before using address ");
		return DescriptorAddress;
	}

	void CreateRTV(D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

	friend class RRenderBackendD3D12;
};
