#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"
#include "DirectXHelper.h"

template<class T>
using TRefCountPtr = Microsoft::WRL::ComPtr<T>;
