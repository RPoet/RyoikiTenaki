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


class RRenderBackend
{
protected:
	String BackendName = TEXT("NONE");
	RRenderCommandList* MainCommandList{};

public:

	RRenderBackend() = default;
	virtual ~RRenderBackend() = default;


	virtual void Init() {}
	virtual void Teardown() {}
	virtual void FunctionalityTestRender() {};

	void SetBackendName(const String& NAME)
	{
		BackendName = NAME;
	}

	const String& GetBackendName()
	{
		return BackendName;
	}

	RRenderCommandList* GetMainCommandList()
	{
		return MainCommandList;
	}

	virtual RDynamicBuffer* GetGlobalDynamicBuffer() { return nullptr;  }
};

