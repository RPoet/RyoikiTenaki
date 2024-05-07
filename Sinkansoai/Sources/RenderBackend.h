#pragma once
#include "Definitions.h"

class IRenderBackend
{
private:
	String BackendName = TEXT("NONE");

public:

	IRenderBackend() = default;
	virtual ~IRenderBackend() = default;

	virtual void Init() {}
	virtual void Exit() {}

	void SetBackendName(const String& NAME)
	{
		BackendName = NAME;
	}

	const String& GetBackendName()
	{
		return BackendName;
	}
};

