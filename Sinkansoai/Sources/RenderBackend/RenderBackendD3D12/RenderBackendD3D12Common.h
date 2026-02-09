#pragma once
#define USE_PIX 1

#include "../../PlatformDefinitions.h"
#include "../RenderBackend.h"
#include "../RenderResource.h"


#include <dxgi1_6.h>
#include <d3d12.h>
#include <pix3.h>
#include "d3dx12.h"
#include "DirectXHelper.h"

constexpr uint32 D3D12MaxConstantBufferSize = 65536u;

template<class T>
using TRefCountPtr = Microsoft::WRL::ComPtr<T>;

extern uint32 GetPerFormatPixelSizeInBytes(DXGI_FORMAT Format);

template<class T, class U>
T* CastAsD3D12(U* Object)
{
	return reinterpret_cast<T*>(Object);
}

class RVertexBuffer;
class RIndexBuffer;
class RTexture;
class RGraphicsPipeline;

inline D3D12_RESOURCE_STATES ToD3D12State(ResourceState State)
{
	switch (State)
	{
	case ResourceState::Common: return D3D12_RESOURCE_STATE_COMMON;
	case ResourceState::RenderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
	case ResourceState::DepthWrite: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case ResourceState::DepthRead: return D3D12_RESOURCE_STATE_DEPTH_READ;
	case ResourceState::ShaderResource: return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	case ResourceState::CopyDest: return D3D12_RESOURCE_STATE_COPY_DEST;
	case ResourceState::CopySource: return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case ResourceState::Present: return D3D12_RESOURCE_STATE_PRESENT;
	case ResourceState::GenericRead: return D3D12_RESOURCE_STATE_GENERIC_READ;
	case ResourceState::UnorderedAccess: return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	default: return D3D12_RESOURCE_STATE_COMMON;
	}
}

inline D3D12_RESOURCE_BARRIER_FLAGS ToD3D12BarrierFlags(ResourceBarrierFlag Flags)
{
	switch (Flags)
	{
	case ResourceBarrierFlag::BeginOnly: return D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
	case ResourceBarrierFlag::EndOnly: return D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
	case ResourceBarrierFlag::None:
	default:
		return D3D12_RESOURCE_BARRIER_FLAG_NONE;
	}
}

inline ID3D12Resource* ToD3D12Resource(RRenderResource* Resource)
{
	if (!Resource)
	{
		return nullptr;
	}
	return reinterpret_cast<ID3D12Resource*>(Resource->GetUnderlyingResource());
}

inline D3D12_RESOURCE_BARRIER ToD3D12Barrier(const ResourceBarrier& Barrier)
{
	D3D12_RESOURCE_BARRIER Out{};
	Out.Flags = ToD3D12BarrierFlags(Barrier.Flags);

	switch (Barrier.Type)
	{
	case ResourceBarrierType::Transition:
		Out.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		Out.Transition.pResource = ToD3D12Resource(Barrier.Transition.Resource);
		Out.Transition.StateBefore = ToD3D12State(Barrier.Transition.StateBefore);
		Out.Transition.StateAfter = ToD3D12State(Barrier.Transition.StateAfter);
		Out.Transition.Subresource = Barrier.Transition.Subresource;
		break;
	case ResourceBarrierType::Aliasing:
		Out.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
		Out.Aliasing.pResourceBefore = ToD3D12Resource(Barrier.Aliasing.ResourceBefore);
		Out.Aliasing.pResourceAfter = ToD3D12Resource(Barrier.Aliasing.ResourceAfter);
		break;
	case ResourceBarrierType::UAV:
	default:
		Out.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		Out.UAV.pResource = ToD3D12Resource(Barrier.UAV.Resource);
		break;
	}

	return Out;
}
