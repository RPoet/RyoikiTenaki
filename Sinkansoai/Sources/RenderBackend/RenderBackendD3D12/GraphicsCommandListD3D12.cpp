#include "GraphicsCommandListD3D12.h"
#include "RenderBackendD3D12.h"
#include "ResourceBufferD3D12.h"

BackendCommandListCommon::~BackendCommandListCommon()
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
}

void BackendCommandListCommon::AllocateCommandList(RRenderBackendD3D12& Backend, D3D12_COMMAND_LIST_TYPE CommandListType)
{
	ID3D12Device* Device = Backend.GetDevice();

	{
		HRESULT HR = Device->CreateCommandAllocator(CommandListType, IID_PPV_ARGS(&CommandAllocator));
		verify(HR);
	}

	{
		HRESULT HR = Device->CreateCommandList(0, CommandListType, CommandAllocator, nullptr, IID_PPV_ARGS(&CommandList));
		verify(HR);
	}

	ThrowIfFailed(CommandList->Close());
	cout << "Graphics Command List Allocation Success" << endl;
}

void BackendCommandListCommon::BeginEvent(UINT64 Color, const wchar_t* Name)
{
	PIXBeginEvent(CommandList, Color, Name);
}

void BackendCommandListCommon::EndEvent()
{
	PIXEndEvent(CommandList);
}

void BackendCommandListCommon::Reset()
{
	verify(CommandAllocator->Reset());
	verify(CommandList->Reset(CommandAllocator, nullptr));

	ScratchPad.Release();
};

void BackendCommandListCommon::Close()
{
	verify(CommandList->Close());
};

void BackendCommandListCommon::SumbitResourceBarriers(uint32 NumBarriers, const ResourceBarrier* Barriers)
{
	if (!Barriers || NumBarriers == 0)
	{
		return;
	}

	pmr_vector<D3D12_RESOURCE_BARRIER> NativeBarriers{ &ScratchPad() };
	NativeBarriers.reserve(NumBarriers);

	for (uint32 i = 0; i < NumBarriers; ++i)
	{
		NativeBarriers.push_back(ToD3D12Barrier(Barriers[i]));
	}

	CommandList->ResourceBarrier(static_cast<UINT>(NativeBarriers.size()), NativeBarriers.data());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RGraphicsCommandListD3D12::~RGraphicsCommandListD3D12()
{
	cout << "Graphics Command List Deleted" << endl;
}

void RGraphicsCommandListD3D12::AllocateCommandList(RRenderBackendD3D12& Backend)
{
	UnderlyingCommandList.AllocateCommandList(Backend, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void RGraphicsCommandListD3D12::Reset()
{
	UnderlyingCommandList.Reset();
}

void RGraphicsCommandListD3D12::Close()
{
	UnderlyingCommandList.Close();
}

void RGraphicsCommandListD3D12::SumbitResourceBarriers(uint32 NumBarriers, const ResourceBarrier* Barriers)
{
	UnderlyingCommandList.SumbitResourceBarriers(NumBarriers, Barriers);
}

void RGraphicsCommandListD3D12::SetViewports(uint32 NumViewports, const D3D12_VIEWPORT* Viewports)
{
	UnderlyingCommandList()->RSSetViewports(NumViewports, Viewports);
}

void RGraphicsCommandListD3D12::SetScissorRects(uint32 NumRects, const D3D12_RECT* Rects)
{
	UnderlyingCommandList()->RSSetScissorRects(NumRects, Rects);
}

void RGraphicsCommandListD3D12::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const float Color[4], uint32 NumRects, const D3D12_RECT* Rects)
{
	UnderlyingCommandList()->ClearRenderTargetView(RenderTargetView, Color, NumRects, Rects);
}

void RGraphicsCommandListD3D12::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags, float Depth, uint8 Stencil, uint32 NumRects, const D3D12_RECT* Rects)
{
	UnderlyingCommandList()->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, Stencil, NumRects, Rects);
}

void RGraphicsCommandListD3D12::OMSetRenderTargets(uint32 NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* RenderTargetDescriptors, bool bSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* DepthStencilDescriptor)
{
	UnderlyingCommandList()->OMSetRenderTargets(NumRenderTargetDescriptors, RenderTargetDescriptors, bSingleHandleToDescriptorRange, DepthStencilDescriptor);
}

void RGraphicsCommandListD3D12::SetDescriptorHeaps(uint32 NumDescriptorHeaps, ID3D12DescriptorHeap* const* DescriptorHeaps)
{
	UnderlyingCommandList()->SetDescriptorHeaps(NumDescriptorHeaps, DescriptorHeaps);
}

void RGraphicsCommandListD3D12::SetGraphicsRootDescriptorTable(uint32 RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
	UnderlyingCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, BaseDescriptor);
}

void RGraphicsCommandListD3D12::SetGraphicsRoot32BitConstants(uint32 RootParameterIndex, uint32 Num32BitValuesToSet, const void* SrcData, uint32 DestOffsetIn32BitValues)
{
	UnderlyingCommandList()->SetGraphicsRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, SrcData, DestOffsetIn32BitValues);
}

void RGraphicsCommandListD3D12::SetGraphicsRoot32BitConstant(uint32 RootParameterIndex, uint32 SrcData, uint32 DestOffsetIn32BitValues)
{
	UnderlyingCommandList()->SetGraphicsRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
}

void RGraphicsCommandListD3D12::SetGraphicsPipeline(RGraphicsPipeline& Pipeline)
{
	UnderlyingCommandList()->SetGraphicsRootSignature(Pipeline.GetRootSignature().Get());
	UnderlyingCommandList()->SetPipelineState(Pipeline.GetPipelineStateObject().Get());
}

void RGraphicsCommandListD3D12::SetPrimitiveTopology(PrimitiveType Type)
{
	UnderlyingCommandList()->IASetPrimitiveTopology(static_cast<D3D12_PRIMITIVE_TOPOLOGY>(Type));
}

void RGraphicsCommandListD3D12::SetVertexBuffer(uint32 Slot, RVertexBuffer* Buffer)
{
	auto& BufferD3D12 = *CastAsD3D12<RVertexBufferD3D12>(Buffer);
	UnderlyingCommandList()->IASetVertexBuffers(Slot, 1, &BufferD3D12.GetVertexBufferView());
}

void RGraphicsCommandListD3D12::SetIndexBuffer(RIndexBuffer* Buffer)
{
	auto& BufferD3D12 = *CastAsD3D12<RIndexBufferD3D12>(Buffer);
	UnderlyingCommandList()->IASetIndexBuffer(&BufferD3D12.GetIndexBufferView());
}

void RGraphicsCommandListD3D12::DrawIndexedInstanced(uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, int32 BaseVertexLocation, uint32 StartInstanceLocation)
{
	UnderlyingCommandList()->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void RGraphicsCommandListD3D12::DrawInstanced(uint32 VertexCountPerInstance, uint32 InstanceCount, uint32 StartVertexLocation, uint32 StartInstanceLocation)
{
	UnderlyingCommandList()->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

// This might be... moved to the somewhere like CopyCommandlistBase...
void RGraphicsCommandListD3D12::CopyTexture(void* pData, ID3D12Resource* Dest, ID3D12Resource* UploadHeap, uint32 TextureWidth, uint32 Height, uint32 PixelSizeInBytes)
{
	D3D12_SUBRESOURCE_DATA TextureData = {};
	TextureData.pData = pData;
	TextureData.RowPitch = TextureWidth * PixelSizeInBytes;
	TextureData.SlicePitch = TextureData.RowPitch * Height;

	UpdateSubresources(UnderlyingCommandList(), Dest, UploadHeap, 0, 0, 1, &TextureData);

	auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Dest, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	UnderlyingCommandList()->ResourceBarrier(1, &Barrier);
}

void RGraphicsCommandListD3D12::BeginEvent(UINT64 Color, const wchar_t* Name)
{
	UnderlyingCommandList.BeginEvent(Color, Name);
}

void RGraphicsCommandListD3D12::EndEvent()
{
	UnderlyingCommandList.EndEvent();
}
