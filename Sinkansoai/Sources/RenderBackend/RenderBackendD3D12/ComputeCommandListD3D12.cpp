#include "ComputeCommandListD3D12.h"
#include "RenderBackendD3D12.h"

RComputeCommandListD3D12::~RComputeCommandListD3D12()
{
	cout << "Compute Command List Deleted" << endl;
}

void RComputeCommandListD3D12::AllocateCommandList(RRenderBackendD3D12& Backend)
{
	UnderlyingCommandList.AllocateCommandList(Backend, D3D12_COMMAND_LIST_TYPE_COMPUTE);
}

void RComputeCommandListD3D12::Reset()
{
	UnderlyingCommandList.Reset();
}

void RComputeCommandListD3D12::Close()
{
	UnderlyingCommandList.Close();
}

void RComputeCommandListD3D12::BeginEvent(UINT64 Color, const wchar_t* Name)
{
	UnderlyingCommandList.BeginEvent(Color, Name);
}

void RComputeCommandListD3D12::EndEvent()
{
	UnderlyingCommandList.EndEvent();
}

void RComputeCommandListD3D12::SumbitResourceBarriers(uint32 NumBarriers, const ResourceBarrier* Barriers)
{
	UnderlyingCommandList.SumbitResourceBarriers(NumBarriers, Barriers);
}

void RComputeCommandListD3D12::SetDescriptorHeaps(uint32 NumDescriptorHeaps, ID3D12DescriptorHeap* const* DescriptorHeaps)
{
	UnderlyingCommandList()->SetDescriptorHeaps(NumDescriptorHeaps, DescriptorHeaps);
}

void RComputeCommandListD3D12::SetComputeRootDescriptorTable(uint32 RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
	UnderlyingCommandList()->SetComputeRootDescriptorTable(RootParameterIndex, BaseDescriptor);
}

void RComputeCommandListD3D12::SetComputeRoot32BitConstants(uint32 RootParameterIndex, uint32 Num32BitValuesToSet, const void* SrcData, uint32 DestOffsetIn32BitValues)
{
	UnderlyingCommandList()->SetComputeRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, SrcData, DestOffsetIn32BitValues);
}

void RComputeCommandListD3D12::SetComputeRoot32BitConstant(uint32 RootParameterIndex, uint32 SrcData, uint32 DestOffsetIn32BitValues)
{
	UnderlyingCommandList()->SetComputeRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
}

void RComputeCommandListD3D12::Dispatch(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ)
{
	UnderlyingCommandList()->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}
