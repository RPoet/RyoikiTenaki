#include "CopyCommandListD3D12.h"
#include "RenderBackendD3D12.h"
#include "TextureD3D12.h"

RCopyCommandListD3D12::~RCopyCommandListD3D12()
{
	cout << "Copy Command List Deleted" << endl;
}

void RCopyCommandListD3D12::AllocateCommandList(RRenderBackendD3D12& Backend)
{
	UnderlyingCommandList.AllocateCommandList(Backend, D3D12_COMMAND_LIST_TYPE_COPY);
}

void RCopyCommandListD3D12::Reset()
{
	UnderlyingCommandList.Reset();
}

void RCopyCommandListD3D12::Close()
{
	UnderlyingCommandList.Close();
}

void RCopyCommandListD3D12::BeginEvent(UINT64 Color, const wchar_t* Name)
{
	UnderlyingCommandList.BeginEvent(Color, Name);
}

void RCopyCommandListD3D12::EndEvent()
{
	UnderlyingCommandList.EndEvent();
}

void RCopyCommandListD3D12::SumbitResourceBarriers(uint32 NumBarriers, const ResourceBarrier* Barriers)
{
	UnderlyingCommandList.SumbitResourceBarriers(NumBarriers, Barriers);
}

void RCopyCommandListD3D12::CopyBufferRegion(ID3D12Resource* Dest, UINT64 DestOffset, ID3D12Resource* Src, UINT64 SrcOffset, UINT64 NumBytes)
{
	UnderlyingCommandList()->CopyBufferRegion(Dest, DestOffset, Src, SrcOffset, NumBytes);
}

void RCopyCommandListD3D12::CopyTexture(void* pData, ID3D12Resource* Dest, ID3D12Resource* UploadHeap, uint32 TextureWidth, uint32 Height, uint32 PixelSizeInBytes)
{
	D3D12_SUBRESOURCE_DATA TextureData = {};
	TextureData.pData = pData;
	TextureData.RowPitch = TextureWidth * PixelSizeInBytes;
	TextureData.SlicePitch = TextureData.RowPitch * Height;

	UpdateSubresources(UnderlyingCommandList(), Dest, UploadHeap, 0, 0, 1, &TextureData);

	auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Dest, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	UnderlyingCommandList()->ResourceBarrier(1, &Barrier);
}
