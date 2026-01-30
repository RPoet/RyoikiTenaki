#include "RenderBackendVulkan.h"

RRenderBackendVulkan::RRenderBackendVulkan()
	: NullDynamicBuffers{ RNullDynamicBuffer(TEXT("VulkanNullBuffer0")), RNullDynamicBuffer(TEXT("VulkanNullBuffer1")) }
{
	SetBackendName(TEXT("VULKAN"));
	MainGraphicsCommandList = &NullGraphicsCommandList;
	MainComputeCommandList = &NullComputeCommandList;
	MainCopyCommandList = &NullCopyCommandList;
}

void RRenderBackendVulkan::Init()
{
	cout << "Vulkan backend stub selected (not implemented yet)" << endl;
}

void RRenderBackendVulkan::Teardown()
{
	cout << "Vulkan backend teardown" << endl;
}

void RRenderBackendVulkan::RenderBegin()
{
}

void RRenderBackendVulkan::RenderFinish()
{
}

RDynamicBuffer* RRenderBackendVulkan::GetGlobalDynamicBuffer(uint32 Index)
{
	if (Index >= 2)
	{
		return nullptr;
	}

	return &NullDynamicBuffers[Index];
}
