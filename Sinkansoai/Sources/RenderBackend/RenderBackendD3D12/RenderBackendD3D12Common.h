#pragma once
#include "../../PlatformDefinitions.h"
#include "../RenderBackend.h"
#include "../RenderResource.h"

#include <dxgi1_6.h>
#include <d3d12.h>
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
