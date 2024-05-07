#pragma once

#include "../RenderBackend.h"
#include "RenderBackendD3D12Common.h"

class RRenderBackendD3D12 : public IRenderBackend
{
private:
	TRefCountPtr< ID3D12Device > Device;

public:
	RRenderBackendD3D12();
	virtual ~RRenderBackendD3D12();

	virtual void Init() override;
	virtual void Exit() override;
};