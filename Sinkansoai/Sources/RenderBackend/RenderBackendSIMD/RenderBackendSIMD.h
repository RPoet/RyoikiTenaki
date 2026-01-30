#pragma once
#include "../../Singleton.h"
#include "../RenderBackend.h"
#include "../RenderBackendNull.h"

class RRenderBackendSIMD : public RRenderBackend, public Singleton<RRenderBackendSIMD>
{
private:
	RNullGraphicsCommandList NullGraphicsCommandList;
	RNullComputeCommandList NullComputeCommandList;
	RNullCopyCommandList NullCopyCommandList;
	RNullDynamicBuffer NullDynamicBuffers[2];

public:
	RRenderBackendSIMD();
	virtual ~RRenderBackendSIMD() = default;

	void Init() override;
	void Teardown() override;
	void RenderBegin() override;
	void RenderFinish() override;

	RDynamicBuffer* GetGlobalDynamicBuffer(uint32 Index) override;
};
