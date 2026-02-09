#pragma once
#include "../PlatformDefinitions.h"
#include "ResourceBarrier.h"

#include <d3d12.h>

using PrimitiveType = uint32;

class RGraphicsPipeline;
class RVertexBuffer;
class RIndexBuffer;

enum CommandListType
{
	Graphics,
	Compute,
	Copy,
	NumCommandList
};

class BackendCommandList
{
public:
	virtual ~BackendCommandList() = default;

	virtual void Reset() = 0;
	virtual void Close() = 0;
	virtual void BeginEvent(UINT64 Color, const wchar_t* Name) = 0;
	virtual void EndEvent() = 0;

	virtual void SumbitResourceBarriers(uint32 NumBarriers, const ResourceBarrier* Barriers) = 0;
};

class RGraphicsCommandList : public BackendCommandList
{
public:
	RGraphicsCommandList() = default;
	virtual ~RGraphicsCommandList() = default;

	virtual void SetViewports(uint32 NumViewports, const D3D12_VIEWPORT* Viewports) = 0;
	virtual void SetScissorRects(uint32 NumRects, const D3D12_RECT* Rects) = 0;

	virtual void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const float Color[4], uint32 NumRects, const D3D12_RECT* Rects) = 0;
	virtual void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags, float Depth, uint8 Stencil, uint32 NumRects, const D3D12_RECT* Rects) = 0;
	virtual void OMSetRenderTargets(uint32 NumRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* RenderTargetDescriptors, bool bSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* DepthStencilDescriptor) = 0;

	virtual void SetDescriptorHeaps(uint32 NumDescriptorHeaps, ID3D12DescriptorHeap* const* DescriptorHeaps) = 0;
	virtual void SetGraphicsRootDescriptorTable(uint32 RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) = 0;
	virtual void SetGraphicsRoot32BitConstants(uint32 RootParameterIndex, uint32 Num32BitValuesToSet, const void* SrcData, uint32 DestOffsetIn32BitValues) = 0;
	virtual void SetGraphicsRoot32BitConstant(uint32 RootParameterIndex, uint32 SrcData, uint32 DestOffsetIn32BitValues) = 0;

	virtual void SetGraphicsPipeline(RGraphicsPipeline& Pipeline) = 0;
	virtual void SetPrimitiveTopology(PrimitiveType Type) = 0;
	virtual void SetVertexBuffer(uint32 Slot, RVertexBuffer* Buffer) = 0;
	virtual void SetIndexBuffer(RIndexBuffer* Buffer) = 0;

	virtual void DrawIndexedInstanced(uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, int32 BaseVertexLocation, uint32 StartInstanceLocation) = 0;
	virtual void DrawInstanced(uint32 VertexCountPerInstance, uint32 InstanceCount, uint32 StartVertexLocation, uint32 StartInstanceLocation) = 0;

	virtual void CopyTexture(void* pData, ID3D12Resource* Dest, ID3D12Resource* UploadHeap, uint32 TextureWidth, uint32 Height, uint32 PixelSizeInBytes) = 0;
};

class RComputeCommandList : public BackendCommandList
{
public:
	RComputeCommandList() = default;
	virtual ~RComputeCommandList() = default;

	virtual void SetDescriptorHeaps(uint32 NumDescriptorHeaps, ID3D12DescriptorHeap* const* DescriptorHeaps) = 0;
	virtual void SetComputeRootDescriptorTable(uint32 RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) = 0;
	virtual void SetComputeRoot32BitConstants(uint32 RootParameterIndex, uint32 Num32BitValuesToSet, const void* SrcData, uint32 DestOffsetIn32BitValues) = 0;
	virtual void SetComputeRoot32BitConstant(uint32 RootParameterIndex, uint32 SrcData, uint32 DestOffsetIn32BitValues) = 0;


	virtual void Dispatch(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ) = 0;

};

class RCopyCommandList : public BackendCommandList
{
public:
	RCopyCommandList() = default;
	virtual ~RCopyCommandList() = default;

	virtual void CopyBufferRegion(ID3D12Resource* Dest, UINT64 DestOffset, ID3D12Resource* Src, UINT64 SrcOffset, UINT64 NumBytes) = 0;
	virtual void CopyTexture(void* pData, ID3D12Resource* Dest, ID3D12Resource* UploadHeap, uint32 TextureWidth, uint32 Height, uint32 PixelSizeInBytes) = 0;
};
