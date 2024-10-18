#pragma once
#include "../../PlatformDefinitions.h"
#include "../RenderResource.h"

#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"
#include "DirectXHelper.h"

constexpr uint32 D3D12MaxConstantBufferSize = 65536u;

template<class T>
using TRefCountPtr = Microsoft::WRL::ComPtr<T>;

