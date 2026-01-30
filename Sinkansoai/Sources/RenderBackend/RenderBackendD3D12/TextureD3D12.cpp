#include "TextureD3D12.h"
#include "RenderBackendD3D12.h"

uint32 GetPerFormatPixelSizeInBytes(DXGI_FORMAT Format)
{
	switch (Format)
	{
	case DXGI_FORMAT_UNKNOWN:
		return 0;
		break;
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		return 4 * 4;
		break;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return 4 * 4;
		break;
	case DXGI_FORMAT_R32G32B32A32_UINT:
		return 4 * 4;
		break;
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 4 * 4;
		break;
	case DXGI_FORMAT_R32G32B32_TYPELESS:
		return 4 * 3;
		break;
	case DXGI_FORMAT_R32G32B32_FLOAT:
		return 4 * 3;
		break;
	case DXGI_FORMAT_R32G32B32_UINT:
		return 4 * 3;
		break;
	case DXGI_FORMAT_R32G32B32_SINT:
		return 4 * 3;
		break;
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		return 2 * 4;
		break;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return 2 * 4;
		break;
	case DXGI_FORMAT_R16G16B16A16_UNORM:
		return 2 * 4;
		break;
	case DXGI_FORMAT_R16G16B16A16_UINT:
		return 2 * 4;
		break;
	case DXGI_FORMAT_R16G16B16A16_SNORM:
		return 2 * 4;
		break;
	case DXGI_FORMAT_R16G16B16A16_SINT:
		return 2 * 4;
		break;
	case DXGI_FORMAT_R32G32_TYPELESS:
		return 4 * 2;
		break;
	case DXGI_FORMAT_R32G32_FLOAT:
		return 4 * 2;
		break;
	case DXGI_FORMAT_R32G32_UINT:
		return 4 * 2;
		break;
	case DXGI_FORMAT_R32G32_SINT:
		return 4 * 2;
		break;
	case DXGI_FORMAT_R32G8X24_TYPELESS:
		return 4 * 2;
		break;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return 4 * 2;
		break;
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		return 4 * 2;
		break;
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return 4 * 2;
		break;
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R10G10B10A2_UINT:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R11G11B10_FLOAT:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R8G8B8A8_UINT:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R8G8B8A8_SNORM:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R8G8B8A8_SINT:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R16G16_TYPELESS:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R16G16_FLOAT:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R16G16_UNORM:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R16G16_UINT:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R16G16_SNORM:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R16G16_SINT:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R32_TYPELESS:
		return 4 * 1;
		break;
	case DXGI_FORMAT_D32_FLOAT:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R32_FLOAT:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R32_UINT:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R32_SINT:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R24G8_TYPELESS:
		return 4 * 1;
		break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		return 4 * 1;
		break;
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return 4 * 1;
		break;
	case DXGI_FORMAT_R8G8_TYPELESS:
		return 1 * 2;
		break;
	case DXGI_FORMAT_R8G8_UNORM:
		return 1 * 2;
		break;
	case DXGI_FORMAT_R8G8_UINT:
		return 1 * 2;
		break;
	case DXGI_FORMAT_R8G8_SNORM:
		return 1 * 2;
		break;
	case DXGI_FORMAT_R8G8_SINT:
		return 1 * 2;
		break;
	case DXGI_FORMAT_R16_TYPELESS:
		return 2;
		break;
	case DXGI_FORMAT_R16_FLOAT:
		return 2;
		break;
	case DXGI_FORMAT_D16_UNORM:
		return 2;
		break;
	case DXGI_FORMAT_R16_UNORM:
		return 2;
		break;
	case DXGI_FORMAT_R16_UINT:
		return 2;
		break;
	case DXGI_FORMAT_R16_SNORM:
		return 2;
		break;
	case DXGI_FORMAT_R16_SINT:
		return 2;
		break;
	case DXGI_FORMAT_R8_TYPELESS:
		return 1;
		break;
	case DXGI_FORMAT_R8_UNORM:
		return 1;
		break;
	case DXGI_FORMAT_R8_UINT:
		return 1;
		break;
	case DXGI_FORMAT_R8_SNORM:
		return 1;
		break;
	case DXGI_FORMAT_R8_SINT:
		return 1;
		break;
	case DXGI_FORMAT_A8_UNORM:
		return 1;
		break;
	case DXGI_FORMAT_R1_UNORM:
		return 1;
		break;
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		return 4;
		break;
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
		return 4;
		break;
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
		return 4;
		break;
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
		assert(false && " Not implemented yet ");
		break;
	case DXGI_FORMAT_B5G6R5_UNORM:
		return 2;
		break;
	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return 2;
		break;
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		return 4;
		break;
	case DXGI_FORMAT_B8G8R8X8_UNORM:
		return 4;
		break;
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		return 4;
		break;
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		return 4;
		break;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return 4;
		break;
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		return 4;
		break;
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return 4;
		break;
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
	case DXGI_FORMAT_AYUV:
	case DXGI_FORMAT_Y410:
	case DXGI_FORMAT_Y416:
	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
	case DXGI_FORMAT_420_OPAQUE:
	case DXGI_FORMAT_YUY2:
	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
	case DXGI_FORMAT_NV11:
	case DXGI_FORMAT_AI44:
	case DXGI_FORMAT_IA44:
	case DXGI_FORMAT_P8:
		assert(false && " Not implemented yet ");
		break;
	case DXGI_FORMAT_A8P8:
		return 2;
		break;
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		return 2;
		break;
	case DXGI_FORMAT_P208:
	case DXGI_FORMAT_V208:
	case DXGI_FORMAT_V408:
	case DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE:
	case DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE:
	case DXGI_FORMAT_FORCE_UINT:
		assert(false && " Not implemented yet ");
		break;
	}

	assert(false && " Not implemented yet ");
	return 0;
}

void RTexture2DD3D12::AllocateResource()
{
	RRenderResource::AllocateResource();

	UnderlyingResource = Backend.CreateTexture2DResource(Name.c_str(), Flag, Format, Width, Height);

	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // change this function of the inputs.
	SRVDesc.Format = Format;
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = NumMips;
}


void RTexture2DD3D12::StreamTexture(void* pData)
{
	const UINT64 UploadBufferSize = GetRequiredIntermediateSize(UnderlyingResource.Get(), 0, 1);
	auto&& UploadHeap = Backend.CreateUploadHeap(UploadBufferSize);
	Backend.GetMainGraphicsCommandList().CopyTexture(pData, UnderlyingResource.Get(), UploadHeap.Get(), Width, Height, PixelSizeInBytes);
}


void RRenderTargetD3D12::AllocateResource()
{
	RRenderResource::AllocateResource();

	UnderlyingResource = Backend.CreateRenderTargetResource(Name.c_str(), Flag, Format, Width, Height);

	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // Check
	SRVDesc.Format = Format;
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = NumMips;
}

void RRenderTargetD3D12::CreateRTV(D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	Backend.GetDevice()->CreateRenderTargetView(GetUnderlyingResource(), nullptr, DestDescriptor);
	bRTVGenerated = true;
	DescriptorAddress = DestDescriptor;
}
