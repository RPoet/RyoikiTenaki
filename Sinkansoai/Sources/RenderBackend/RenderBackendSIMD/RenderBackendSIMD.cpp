#include "RenderBackendSIMD.h"

RRenderBackendSIMD::RRenderBackendSIMD()
	: NullDynamicBuffers{ RNullDynamicBuffer(TEXT("SimdNullBuffer0")), RNullDynamicBuffer(TEXT("SimdNullBuffer1")) }
{
	SetBackendName(TEXT("SIMD"));
	MainGraphicsCommandList = &NullGraphicsCommandList;
	MainComputeCommandList = &NullComputeCommandList;
	MainCopyCommandList = &NullCopyCommandList;
}

void RRenderBackendSIMD::Init()
{
	cout << "SIMD backend stub selected (not implemented yet)" << endl;
}

void RRenderBackendSIMD::Teardown()
{
	cout << "SIMD backend teardown" << endl;
}

void RRenderBackendSIMD::RenderBegin()
{
}

void RRenderBackendSIMD::RenderFinish()
{
}

RDynamicBuffer* RRenderBackendSIMD::GetGlobalDynamicBuffer(uint32 Index)
{
	if (Index >= 2)
	{
		return nullptr;
	}

	return &NullDynamicBuffers[Index];
}
