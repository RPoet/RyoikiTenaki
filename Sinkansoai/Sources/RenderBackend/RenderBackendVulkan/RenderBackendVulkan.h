#pragma once
#include "../../Singleton.h"
#include "../RenderBackend.h"
#include "../RenderBackendNull.h"

// Platform-specific Vulkan surface extension
#if PLATFORM_WINDOWS
    #if !defined(VK_USE_PLATFORM_WIN32_KHR)
        #define VK_USE_PLATFORM_WIN32_KHR
    #endif
#elif PLATFORM_ANDROID
    #if !defined(VK_USE_PLATFORM_ANDROID_KHR)
        #define VK_USE_PLATFORM_ANDROID_KHR
    #endif
#endif

#include <vulkan/vulkan.h>

class RRenderBackendVulkan : public RRenderBackend, public Singleton<RRenderBackendVulkan>
{
private:
    // Core Vulkan
    VkInstance Instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
    VkDevice Device = VK_NULL_HANDLE;
    VkQueue GraphicsQueue = VK_NULL_HANDLE;
    VkQueue ComputeQueue = VK_NULL_HANDLE; // Often same as graphics
    VkQueue CopyQueue = VK_NULL_HANDLE;    // Often same or dedicated

    // SwapChain
    VkSurfaceKHR Surface = VK_NULL_HANDLE;
    VkSwapchainKHR SwapChain = VK_NULL_HANDLE;
    vector<VkImage> SwapChainImages;
    vector<VkImageView> SwapChainImageViews;
    VkFormat SwapChainImageFormat;
    VkExtent2D SwapChainExtent;
    
    // Command Pools
    VkCommandPool CommandPool = VK_NULL_HANDLE; // One per thread/queue usually

    // Synchronization
    static const int MAX_FRAMES_IN_FLIGHT = 2;
    VkSemaphore ImageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore RenderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence InFlightFences[MAX_FRAMES_IN_FLIGHT];
    uint32 CurrentFrame = 0;

    // Temporary Null Objects (to satisfy interface for now)
	RNullGraphicsCommandList NullGraphicsCommandList;
	RNullComputeCommandList NullComputeCommandList;
	RNullCopyCommandList NullCopyCommandList;
	RNullDynamicBuffer NullDynamicBuffers[2];

public:
	RRenderBackendVulkan();
	virtual ~RRenderBackendVulkan();

	void Init() override;
	void Teardown() override;
	void RenderBegin() override;
	void RenderFinish() override;

	RDynamicBuffer* GetGlobalDynamicBuffer(uint32 Index) override;
    
    // Vulkan Accessors
    VkDevice GetDevice() { return Device; }
    VkPhysicalDevice GetPhysicalDevice() { return PhysicalDevice; }
};
