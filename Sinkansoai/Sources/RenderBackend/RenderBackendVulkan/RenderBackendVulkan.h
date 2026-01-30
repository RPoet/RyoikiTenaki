#pragma once
#include "../../Singleton.h"
#include "../RenderBackend.h"
#include "../RenderBackendNull.h"

class RRenderBackendVulkan : public RRenderBackend, public Singleton<RRenderBackendVulkan>
{
private:
	RNullGraphicsCommandList NullGraphicsCommandList;
	RNullComputeCommandList NullComputeCommandList;
	RNullCopyCommandList NullCopyCommandList;
	RNullDynamicBuffer NullDynamicBuffers[2];

public:
	RRenderBackendVulkan();
	virtual ~RRenderBackendVulkan() = default;

	void Init() override;
	void Teardown() override;
	void RenderBegin() override;
	void RenderFinish() override;

	RDynamicBuffer* GetGlobalDynamicBuffer(uint32 Index) override;
};
