#include "RenderBackendCompute.h"

RRenderBackendCompute::RRenderBackendCompute()
	: NullDynamicBuffers{ RNullDynamicBuffer(TEXT("ComputeNullBuffer0")), RNullDynamicBuffer(TEXT("ComputeNullBuffer1")) }
{
	SetBackendName(TEXT("COMPUTE"));
	MainGraphicsCommandList = &NullGraphicsCommandList;
	MainComputeCommandList = &NullComputeCommandList;
	MainCopyCommandList = &NullCopyCommandList;
}

void RRenderBackendCompute::Init()
{
	cout << "Compute backend stub selected (not implemented yet)" << endl;
}

void RRenderBackendCompute::Teardown()
{
	cout << "Compute backend teardown" << endl;
}

void RRenderBackendCompute::RenderBegin()
{
}

void RRenderBackendCompute::RenderFinish()
{
}

RDynamicBuffer* RRenderBackendCompute::GetGlobalDynamicBuffer(uint32 Index)
{
	if (Index >= 2)
	{
		return nullptr;
	}

	return &NullDynamicBuffers[Index];
}
