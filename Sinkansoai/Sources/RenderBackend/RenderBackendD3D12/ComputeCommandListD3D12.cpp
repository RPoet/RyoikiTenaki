#include "ComputeCommandListD3D12.h"
#include "RenderBackendD3D12.h"

RComputeCommandListD3D12::~RComputeCommandListD3D12()
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

	cout << "Compute Command List Deleted" << endl;
}

void RComputeCommandListD3D12::AllocateCommandList(RRenderBackendD3D12& Backend)
{
	ID3D12Device* Device = Backend.GetDevice();

	{
		HRESULT HR = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&CommandAllocator));
		verify(HR);
	}

	{
		HRESULT HR = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, CommandAllocator, nullptr, IID_PPV_ARGS(&CommandList));
		verify(HR);
	}

	ThrowIfFailed(CommandList->Close());
	cout << "Compute Command List Allocation Success" << endl;
}

void RComputeCommandListD3D12::Reset()
{
	verify(CommandAllocator->Reset());
	verify(CommandList->Reset(CommandAllocator, nullptr));
}

void RComputeCommandListD3D12::Close()
{
	verify(CommandList->Close());
}

void RComputeCommandListD3D12::ResourceBarrier(uint32 NumBarriers, const D3D12_RESOURCE_BARRIER* Barriers)
{
	CommandList->ResourceBarrier(NumBarriers, Barriers);
}

void RComputeCommandListD3D12::SetDescriptorHeaps(uint32 NumDescriptorHeaps, ID3D12DescriptorHeap* const* DescriptorHeaps)
{
	CommandList->SetDescriptorHeaps(NumDescriptorHeaps, DescriptorHeaps);
}

void RComputeCommandListD3D12::SetComputeRootDescriptorTable(uint32 RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
	CommandList->SetComputeRootDescriptorTable(RootParameterIndex, BaseDescriptor);
}

void RComputeCommandListD3D12::SetComputeRoot32BitConstants(uint32 RootParameterIndex, uint32 Num32BitValuesToSet, const void* SrcData, uint32 DestOffsetIn32BitValues)
{
	CommandList->SetComputeRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, SrcData, DestOffsetIn32BitValues);
}

void RComputeCommandListD3D12::SetComputeRoot32BitConstant(uint32 RootParameterIndex, uint32 SrcData, uint32 DestOffsetIn32BitValues)
{
	CommandList->SetComputeRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
}

void RComputeCommandListD3D12::Dispatch(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ)
{
	CommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void RComputeCommandListD3D12::BeginEvent(UINT64 Color, const wchar_t* Name)
{
	PIXBeginEvent(CommandList, Color, Name);
}

void RComputeCommandListD3D12::EndEvent()
{
	PIXEndEvent(CommandList);
}
