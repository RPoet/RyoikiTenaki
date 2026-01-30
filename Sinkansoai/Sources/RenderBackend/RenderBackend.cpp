#include "RenderBackend.h"
#include "RenderBackendD3D12/RenderBackendD3D12.h"
#include "RenderBackendVulkan/RenderBackendVulkan.h"
#include "RenderBackendSIMD/RenderBackendSIMD.h"
#include "RenderBackendCompute/RenderBackendCompute.h"

#include <algorithm>
#include <cwctype>

RRenderBackend* GBackend = nullptr;

static String NormalizeBackendName(const String& BackendName)
{
	String Upper = BackendName;
	std::transform(Upper.begin(), Upper.end(), Upper.begin(), [](wchar_t Ch)
	{
		return static_cast<wchar_t>(towupper(Ch));
	});

	return Upper;
}

ERenderBackendType BackendTypeFromName(const String& BackendName)
{
	const String Upper = NormalizeBackendName(BackendName);

	if (Upper == TEXT("D3D12") || Upper == TEXT("DX12"))
	{
		return ERenderBackendType::D3D12;
	}

	if (Upper == TEXT("VULKAN") || Upper == TEXT("VK"))
	{
		return ERenderBackendType::Vulkan;
	}

	if (Upper == TEXT("SIMD") || Upper == TEXT("CPU"))
	{
		return ERenderBackendType::SIMD;
	}

	if (Upper == TEXT("COMPUTE") || Upper == TEXT("COMPUTE_SHADER") || Upper == TEXT("CS"))
	{
		return ERenderBackendType::Compute;
	}

	return ERenderBackendType::D3D12;
}

const String& BackendNameFromType(ERenderBackendType BackendType)
{
	static String D3D12Name = TEXT("D3D12");
	static String VulkanName = TEXT("VULKAN");
	static String SIMDName = TEXT("SIMD");
	static String ComputeName = TEXT("COMPUTE");

	switch (BackendType)
	{
	case ERenderBackendType::D3D12:
		return D3D12Name;
	case ERenderBackendType::Vulkan:
		return VulkanName;
	case ERenderBackendType::SIMD:
		return SIMDName;
	case ERenderBackendType::Compute:
		return ComputeName;
	default:
		return D3D12Name;
	}
}

void InitBackend(ERenderBackendType BackendType)
{
	switch (BackendType)
	{
	case ERenderBackendType::D3D12:
		{
			GBackend = &RRenderBackendD3D12::Get();
			cout << "D3D12 backend selected" << endl;
			break;
		}
	case ERenderBackendType::Vulkan:
		{
			GBackend = &RRenderBackendVulkan::Get();
			cout << "Vulkan backend selected" << endl;
			break;
		}
	case ERenderBackendType::SIMD:
		{
			GBackend = &RRenderBackendSIMD::Get();
			cout << "SIMD backend selected" << endl;
			break;
		}
	case ERenderBackendType::Compute:
		{
			GBackend = &RRenderBackendCompute::Get();
			cout << "Compute backend selected" << endl;
			break;
		}
	default:
		break;
	}

	if (GBackend)
	{
		GBackend->Init();
	}
}

void InitBackend(const String& BackendName)
{
	InitBackend(BackendTypeFromName(BackendName));
}

void TeardownBackend()
{
	if (GBackend)
	{
		GBackend->Teardown();
		cout << "Renderbackend correctly tears down" << endl;
	}
	GBackend = nullptr;
}
