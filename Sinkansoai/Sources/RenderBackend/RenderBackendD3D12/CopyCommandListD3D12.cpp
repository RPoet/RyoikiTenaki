#include "CopyCommandListD3D12.h"
#include "RenderBackendD3D12.h"
#include "TextureD3D12.h"

RCopyCommandListD3D12::~RCopyCommandListD3D12()
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

	cout << "Copy Command List Deleted" << endl;
}

void RCopyCommandListD3D12::AllocateCommandList(RRenderBackendD3D12& Backend)
{
	ID3D12Device* Device = Backend.GetDevice();

	{
		HRESULT HR = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&CommandAllocator));
		verify(HR);
	}

	{
		HRESULT HR = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, CommandAllocator, nullptr, IID_PPV_ARGS(&CommandList));
		verify(HR);
	}

	ThrowIfFailed(CommandList->Close());
	cout << "Copy Command List Allocation Success" << endl;
}

void RCopyCommandListD3D12::Reset()
{
	verify(CommandAllocator->Reset());
	verify(CommandList->Reset(CommandAllocator, nullptr));
}

void RCopyCommandListD3D12::Close()
{
	verify(CommandList->Close());
}

void RCopyCommandListD3D12::ResourceBarrier(uint32 NumBarriers, const D3D12_RESOURCE_BARRIER* Barriers)
{
	CommandList->ResourceBarrier(NumBarriers, Barriers);
}

void RCopyCommandListD3D12::CopyBufferRegion(ID3D12Resource* Dest, UINT64 DestOffset, ID3D12Resource* Src, UINT64 SrcOffset, UINT64 NumBytes)
{
	CommandList->CopyBufferRegion(Dest, DestOffset, Src, SrcOffset, NumBytes);
}

void RCopyCommandListD3D12::CopyTexture(void* pData, ID3D12Resource* Dest, ID3D12Resource* UploadHeap, uint32 TextureWidth, uint32 Height, uint32 PixelSizeInBytes)
{
	D3D12_SUBRESOURCE_DATA TextureData = {};
	TextureData.pData = pData;
	TextureData.RowPitch = TextureWidth * PixelSizeInBytes;
	TextureData.SlicePitch = TextureData.RowPitch * Height;

	UpdateSubresources(CommandList, Dest, UploadHeap, 0, 0, 1, &TextureData);

	auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Dest, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandList->ResourceBarrier(1, &Barrier);
}

void RCopyCommandListD3D12::BeginEvent(UINT64 Color, const wchar_t* Name)
{
	PIXBeginEvent(CommandList, Color, Name);
}

void RCopyCommandListD3D12::EndEvent()
{
	PIXEndEvent(CommandList);
}
