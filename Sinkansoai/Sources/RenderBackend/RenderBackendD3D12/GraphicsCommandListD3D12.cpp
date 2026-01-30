#include "GraphicsCommandListD3D12.h"
#include "RenderBackendD3D12.h"
#include "ResourceBufferD3D12.h"

RGraphicsCommandListD3D12::~RGraphicsCommandListD3D12()
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

	cout << "Graphics Command List Deleted" << endl;
}

void RGraphicsCommandListD3D12::AllocateCommandList(RRenderBackendD3D12& Backend)
{
	ID3D12Device* Device = Backend.GetDevice();

	{
		HRESULT HR = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator));
		verify(HR);
	}

	{
		HRESULT HR = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator, nullptr, IID_PPV_ARGS(&CommandList));
		verify(HR);
	}

	ThrowIfFailed(CommandList->Close());
	cout << "Graphics Command List Allocation Success" << endl;
}

void RGraphicsCommandListD3D12::Reset()
{
	verify(CommandAllocator->Reset());
	verify(CommandList->Reset(CommandAllocator, nullptr));
}

void RGraphicsCommandListD3D12::Close()
{
	verify(CommandList->Close());
}

void RGraphicsCommandListD3D12::ResourceBarrier(uint32 NumBarriers, const D3D12_RESOURCE_BARRIER* Barriers)
{
	CommandList->ResourceBarrier(NumBarriers, Barriers);
}

void RGraphicsCommandListD3D12::SetViewports(uint32 NumViewports, const D3D12_VIEWPORT* Viewports)
{
	CommandList->RSSetViewports(NumViewports, Viewports);
}

void RGraphicsCommandListD3D12::SetScissorRects(uint32 NumRects, const D3D12_RECT* Rects)
{
	CommandList->RSSetScissorRects(NumRects, Rects);
}

void RGraphicsCommandListD3D12::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const float Color[4], uint32 NumRects, const D3D12_RECT* Rects)
{
	CommandList->ClearRenderTargetView(RenderTargetView, Color, NumRects, Rects);
}

void RGraphicsCommandListD3D12::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags, float Depth, uint8 Stencil, uint32 NumRects, const D3D12_RECT* Rects)
{
	CommandList->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, Stencil, NumRects, Rects);
}

void RGraphicsCommandListD3D12::OMSetRenderTargets(uint32 NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* RenderTargetDescriptors, bool bSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* DepthStencilDescriptor)
{
	CommandList->OMSetRenderTargets(NumRenderTargetDescriptors, RenderTargetDescriptors, bSingleHandleToDescriptorRange, DepthStencilDescriptor);
}

void RGraphicsCommandListD3D12::SetDescriptorHeaps(uint32 NumDescriptorHeaps, ID3D12DescriptorHeap* const* DescriptorHeaps)
{
	CommandList->SetDescriptorHeaps(NumDescriptorHeaps, DescriptorHeaps);
}

void RGraphicsCommandListD3D12::SetGraphicsRootDescriptorTable(uint32 RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
	CommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, BaseDescriptor);
}

void RGraphicsCommandListD3D12::SetGraphicsRoot32BitConstants(uint32 RootParameterIndex, uint32 Num32BitValuesToSet, const void* SrcData, uint32 DestOffsetIn32BitValues)
{
	CommandList->SetGraphicsRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, SrcData, DestOffsetIn32BitValues);
}

void RGraphicsCommandListD3D12::SetGraphicsRoot32BitConstant(uint32 RootParameterIndex, uint32 SrcData, uint32 DestOffsetIn32BitValues)
{
	CommandList->SetGraphicsRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
}

void RGraphicsCommandListD3D12::SetGraphicsPipeline(RGraphicsPipeline& Pipeline)
{
	CommandList->SetGraphicsRootSignature(Pipeline.GetRootSignature().Get());
	CommandList->SetPipelineState(Pipeline.GetPipelineStateObject().Get());
}

void RGraphicsCommandListD3D12::SetPrimitiveTopology(PrimitiveType Type)
{
	CommandList->IASetPrimitiveTopology(static_cast<D3D12_PRIMITIVE_TOPOLOGY>(Type));
}

void RGraphicsCommandListD3D12::SetVertexBuffer(uint32 Slot, RVertexBuffer* Buffer)
{
	auto& BufferD3D12 = *CastAsD3D12<RVertexBufferD3D12>(Buffer);
	CommandList->IASetVertexBuffers(Slot, 1, &BufferD3D12.GetVertexBufferView());
}

void RGraphicsCommandListD3D12::SetIndexBuffer(RIndexBuffer* Buffer)
{
	auto& BufferD3D12 = *CastAsD3D12<RIndexBufferD3D12>(Buffer);
	CommandList->IASetIndexBuffer(&BufferD3D12.GetIndexBufferView());
}

void RGraphicsCommandListD3D12::DrawIndexedInstanced(uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, int32 BaseVertexLocation, uint32 StartInstanceLocation)
{
	CommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void RGraphicsCommandListD3D12::DrawInstanced(uint32 VertexCountPerInstance, uint32 InstanceCount, uint32 StartVertexLocation, uint32 StartInstanceLocation)
{
	CommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void RGraphicsCommandListD3D12::CopyTexture(void* pData, ID3D12Resource* Dest, ID3D12Resource* UploadHeap, uint32 TextureWidth, uint32 Height, uint32 PixelSizeInBytes)
{
	D3D12_SUBRESOURCE_DATA TextureData = {};
	TextureData.pData = pData;
	TextureData.RowPitch = TextureWidth * PixelSizeInBytes;
	TextureData.SlicePitch = TextureData.RowPitch * Height;

	UpdateSubresources(CommandList, Dest, UploadHeap, 0, 0, 1, &TextureData);

	auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Dest, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandList->ResourceBarrier(1, &Barrier);
}

void RGraphicsCommandListD3D12::BeginEvent(UINT64 Color, const wchar_t* Name)
{
	PIXBeginEvent(CommandList, Color, Name);
}

void RGraphicsCommandListD3D12::EndEvent()
{
	PIXEndEvent(CommandList);
}
