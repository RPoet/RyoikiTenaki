#pragma once
#include "../../Singleton.h"
#include "../RenderBackend.h"
#include "../RenderBackendNull.h"

class RRenderBackendCompute : public RRenderBackend, public Singleton<RRenderBackendCompute>
{
private:
	RNullGraphicsCommandList NullGraphicsCommandList;
	RNullComputeCommandList NullComputeCommandList;
	RNullCopyCommandList NullCopyCommandList;
	RNullDynamicBuffer NullDynamicBuffers[2];

public:
	RRenderBackendCompute();
	virtual ~RRenderBackendCompute() = default;

	void Init() override;
	void Teardown() override;
	void RenderBegin() override;
	void RenderFinish() override;

	RDynamicBuffer* GetGlobalDynamicBuffer(uint32 Index) override;
};
