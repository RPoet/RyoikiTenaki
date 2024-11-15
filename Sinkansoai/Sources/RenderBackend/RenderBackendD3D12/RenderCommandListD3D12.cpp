#include "RenderCommandListD3D12.h"
#include "RenderBackendD3D12.h"

void RRenderCommandListD3D12::AllocateCommandLsit(RRenderBackendD3D12& Backend)
{
	auto Device = Backend.Device;

	{
		HRESULT HR = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator));
		verify(HR);
	}

	{
		HRESULT HR = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator, nullptr, IID_PPV_ARGS(&CommandList));
		verify(HR);
	}

	ThrowIfFailed( CommandList->Close() );
	
	cout << "Command List Allocation Success" << endl;

	bClose = false;
}

void RRenderCommandListD3D12::CopyTexture(void* pData, ID3D12Resource* Dest, ID3D12Resource* UploadHeap, const uint32 TextureWidth,  const uint32 Height, const uint32 PixelSizeInBytes)
{
	D3D12_SUBRESOURCE_DATA TextureData = {};
	TextureData.pData = pData;
	TextureData.RowPitch = TextureWidth * PixelSizeInBytes;
	TextureData.SlicePitch = TextureData.RowPitch * Height;

	UpdateSubresources(CommandList, Dest, UploadHeap, 0, 0, 1, &TextureData);
	
	auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Dest, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	// move this later
	CommandList->ResourceBarrier(1, &Barrier);
}

RRenderCommandListD3D12::~RRenderCommandListD3D12()
{
	if (CommandList)
	{
		CommandList->Release();
	}
	CommandList = nullptr;


	if (CommandAllocator)
	{
		CommandAllocator->Release();
	}
	CommandAllocator = nullptr;


	cout << "Command List Deleted" << endl;
}

void RRenderCommandListD3D12::Reset()
{
	verify(CommandAllocator->Reset());
	verify(CommandList->Reset(CommandAllocator, nullptr));
}

void RRenderCommandListD3D12::Close()
{
	verify(CommandList->Close());
}
