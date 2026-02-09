#include "RenderBackendVulkan.h"

RRenderBackendVulkan::RRenderBackendVulkan()
	: NullDynamicBuffers{ RNullDynamicBuffer(TEXT("VulkanNullBuffer0")), RNullDynamicBuffer(TEXT("VulkanNullBuffer1")) }
{
	SetBackendName(TEXT("VULKAN"));
	MainGraphicsCommandList = &NullGraphicsCommandList;
	MainComputeCommandList = &NullComputeCommandList;
	MainCopyCommandList = &NullCopyCommandList;
}



RRenderBackendVulkan::~RRenderBackendVulkan()
{
    Teardown();
}

void RRenderBackendVulkan::Init()
{
	cout << "Initializing Vulkan Backend..." << endl;
    
    // Application Info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Sinkansoai";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Sinkansoai Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Instance Create Info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Extensions (Need 'VK_KHR_surface', 'VK_KHR_win32_surface' usually)
    const char* extensionNames[] = { "VK_KHR_surface", "VK_KHR_win32_surface" };
    createInfo.enabledExtensionCount = 2;
    createInfo.ppEnabledExtensionNames = extensionNames;
    
    // Validation Layers (hardcoded check for now, good for Debug)
#ifdef _DEBUG
    const char* validationLayers[] = { "VK_LAYER_KHRONOS_validation" };
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = validationLayers;
#else
    createInfo.enabledLayerCount = 0;
#endif

    VkResult result = vkCreateInstance(&createInfo, nullptr, &Instance);
    if (result != VK_SUCCESS)
    {
        cout << "Failed to create Vulkan Instance!" << endl;
        return;
    }
    
    cout << "Vulkan Instance Created Successfully." << endl;
    
    // Physical Device Selection (Pick first one suitable)
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        cout << "failed to find GPUs with Vulkan support!" << endl;
        return;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data());
    PhysicalDevice = devices[0]; // Naive implementation: pick first
    
    // Device Queue Families logic omitted for brevity, assuming index 0 for now in this stub
}

void RRenderBackendVulkan::Teardown()
{
	cout << "Vulkan backend teardown" << endl;
    
    if (Device != VK_NULL_HANDLE) {
        vkDestroyDevice(Device, nullptr);
        Device = VK_NULL_HANDLE;
    }
    
    if (Instance != VK_NULL_HANDLE) {
        vkDestroyInstance(Instance, nullptr);
        Instance = VK_NULL_HANDLE;
    }
}

void RRenderBackendVulkan::RenderBegin()
{
    // Acquire Next Image
}

void RRenderBackendVulkan::RenderFinish()
{
    // Submit Command Buffer
    // Present
}

RDynamicBuffer* RRenderBackendVulkan::GetGlobalDynamicBuffer(uint32 Index)
{
	if (Index >= 2)
	{
		return nullptr;
	}

	return &NullDynamicBuffers[Index];
}
