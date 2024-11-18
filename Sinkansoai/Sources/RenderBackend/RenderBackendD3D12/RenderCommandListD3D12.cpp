#include "RenderCommandListD3D12.h"
#include "RenderBackendD3D12.h"

#include "ResourceBufferD3D12.h"
#include "TextureD3D12.h"

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

void RRenderCommandListD3D12::Reset()
{
	verify(CommandAllocator->Reset());
	verify(CommandList->Reset(CommandAllocator, nullptr));
}

void RRenderCommandListD3D12::Close()
{
	verify(CommandList->Close());
}

void RRenderCommandListD3D12::CopyTexture(void* pData, ID3D12Resource* Dest, ID3D12Resource* UploadHeap, const uint32 TextureWidth, const uint32 Height, const uint32 PixelSizeInBytes)
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

void RRenderCommandListD3D12::SetGraphicsPipeline(RGraphicsPipeline& Pipeline)
{
	CommandList->SetGraphicsRootSignature(Pipeline.GetRootSignature().Get());
	CommandList->SetPipelineState(Pipeline.GetPipelineStateObject().Get());
}

void RRenderCommandListD3D12::SetPrimitiveTopology(PrimitiveType Type)
{
	D3D12_PRIMITIVE_TOPOLOGY Topology = D3D12_PRIMITIVE_TOPOLOGY(Type);
	CommandList->IASetPrimitiveTopology(Topology);
}

void RRenderCommandListD3D12::SetVertexBuffer(uint32 Slot, RVertexBuffer* Buffer)
{
	auto& BufferD3D12 = *CastAsD3D12<RVertexBufferD3D12>(Buffer);
	CommandList->IASetVertexBuffers(Slot, 1, &BufferD3D12.GetVertexBufferView());
}

void RRenderCommandListD3D12::SetIndexBuffer(RIndexBuffer* Buffer)
{
	auto& BufferD3D12 = *CastAsD3D12<RIndexBufferD3D12>(Buffer);
	CommandList->IASetIndexBuffer(&BufferD3D12.GetIndexBufferView());
}

