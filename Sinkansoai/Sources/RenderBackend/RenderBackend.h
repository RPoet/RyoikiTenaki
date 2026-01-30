#pragma once
#include "../Definitions.h"
#include "RenderCommandList.h"
#include "DynamicBuffer.h"

enum EResourceType
{
	RenderBuffer = 1,
	RenderTexture1D = 2,
	RenderTexture2D = 3,
	RenderTexture3D = 4,
	Texture2D = 5
};

enum EResourceFlag
{
	None = 0,
	RenderTarget = 1,
	DepthStencilTarget = 2,
	UnorderedAccess = 3,
	DenyShaderResource = 4,
	AllowSimultanenousAccess = 5,
};

enum class ERenderBackendType : uint8
{
	D3D12,
	Vulkan,
	SIMD,
	Compute
};

ERenderBackendType BackendTypeFromName(const String& BackendName);
const String& BackendNameFromType(ERenderBackendType BackendType);


class RRenderBackend
{
protected:
	String BackendName = TEXT("NONE");
	RGraphicsCommandList* MainGraphicsCommandList{};
	RComputeCommandList* MainComputeCommandList{};
	RCopyCommandList* MainCopyCommandList{};

public:

	RRenderBackend() = default;
	virtual ~RRenderBackend() = default;

	virtual void Init() {};
	virtual void Teardown() {};
	virtual void FunctionalityTestRender(bool bDeferred, uint32 TestInput) {};

	virtual void RenderBegin() {};
	virtual void RenderFinish() {};

	void SetBackendName(const String& NAME)
	{
		BackendName = NAME;
	}

	const String& GetBackendName()
	{
		return BackendName;
	}

	RGraphicsCommandList* GetMainGraphicsCommandList()
	{
		return MainGraphicsCommandList;
	}

	RComputeCommandList* GetMainComputeCommandList()
	{
		return MainComputeCommandList;
	}

	RCopyCommandList* GetMainCopyCommandList()
	{
		return MainCopyCommandList;
	}

	virtual RDynamicBuffer* GetGlobalDynamicBuffer(uint32 Index) { return nullptr;  }
};

