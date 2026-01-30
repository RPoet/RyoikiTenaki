#pragma once
#include "../PlatformDefinitions.h"
#include "RenderCommandList.h"
#include "DynamicBuffer.h"

#include <array>

class RNullDynamicBuffer final : public RDynamicBuffer
{
public:
	RNullDynamicBuffer()
		: RDynamicBuffer(TEXT("NullDynamicBuffer"))
	{
		InitBuffer();
	}

	explicit RNullDynamicBuffer(const String& Name)
		: RDynamicBuffer(Name)
	{
		InitBuffer();
	}

	explicit RNullDynamicBuffer(String&& Name)
		: RDynamicBuffer(std::move(Name))
	{
		InitBuffer();
	}

private:
	static constexpr uint32 kNullDynamicBufferSize = 65536u;

	void InitBuffer()
	{
		Size = kNullDynamicBufferSize;
		Data = Storage.data();
		bInitialized = true;
	}

	std::array<uint8, kNullDynamicBufferSize> Storage{};
};

class RNullGraphicsCommandList final : public RGraphicsCommandList
{
public:
	void Reset() override {}
	void Close() override {}

	void ResourceBarrier(uint32, const D3D12_RESOURCE_BARRIER*) override {}
	void SetViewports(uint32, const D3D12_VIEWPORT*) override {}
	void SetScissorRects(uint32, const D3D12_RECT*) override {}

	void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE, const float[4], uint32, const D3D12_RECT*) override {}
	void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_CLEAR_FLAGS, float, uint8, uint32, const D3D12_RECT*) override {}
	void OMSetRenderTargets(uint32, const D3D12_CPU_DESCRIPTOR_HANDLE*, bool, const D3D12_CPU_DESCRIPTOR_HANDLE*) override {}

	void SetDescriptorHeaps(uint32, ID3D12DescriptorHeap* const*) override {}
	void SetGraphicsRootDescriptorTable(uint32, D3D12_GPU_DESCRIPTOR_HANDLE) override {}
	void SetGraphicsRoot32BitConstants(uint32, uint32, const void*, uint32) override {}
	void SetGraphicsRoot32BitConstant(uint32, uint32, uint32) override {}

	void SetGraphicsPipeline(RGraphicsPipeline&) override {}
	void SetPrimitiveTopology(PrimitiveType) override {}
	void SetVertexBuffer(uint32, RVertexBuffer*) override {}
	void SetIndexBuffer(RIndexBuffer*) override {}

	void DrawIndexedInstanced(uint32, uint32, uint32, int32, uint32) override {}
	void DrawInstanced(uint32, uint32, uint32, uint32) override {}

	void CopyTexture(void*, ID3D12Resource*, ID3D12Resource*, uint32, uint32, uint32) override {}

	void BeginEvent(UINT64, const wchar_t*) override {}
	void EndEvent() override {}
};

class RNullComputeCommandList final : public RComputeCommandList
{
public:
	void Reset() override {}
	void Close() override {}

	void ResourceBarrier(uint32, const D3D12_RESOURCE_BARRIER*) override {}
	void SetDescriptorHeaps(uint32, ID3D12DescriptorHeap* const*) override {}
	void SetComputeRootDescriptorTable(uint32, D3D12_GPU_DESCRIPTOR_HANDLE) override {}
	void SetComputeRoot32BitConstants(uint32, uint32, const void*, uint32) override {}
	void SetComputeRoot32BitConstant(uint32, uint32, uint32) override {}
	void Dispatch(uint32, uint32, uint32) override {}

	void BeginEvent(UINT64, const wchar_t*) override {}
	void EndEvent() override {}
};

class RNullCopyCommandList final : public RCopyCommandList
{
public:
	void Reset() override {}
	void Close() override {}

	void ResourceBarrier(uint32, const D3D12_RESOURCE_BARRIER*) override {}
	void CopyBufferRegion(ID3D12Resource*, UINT64, ID3D12Resource*, UINT64, UINT64) override {}
	void CopyTexture(void*, ID3D12Resource*, ID3D12Resource*, uint32, uint32, uint32) override {}

	void BeginEvent(UINT64, const wchar_t*) override {}
	void EndEvent() override {}
};
