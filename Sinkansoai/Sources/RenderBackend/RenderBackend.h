#pragma once
#include "../Definitions.h"
#include "RenderCommandList.h"

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
};

