#pragma once

#include "defines.h"
#include <vulkan/vulkan_core.h>
// Checks the given expression's return value is OK.
#define VK_CHECK(expr)                                                         \
    {                                                                          \
        FASSERT(expr == VK_SUCCESS);                                           \
    }

typedef struct VulkanDevice {
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    u32 graphicsFamily;
    VkQueue graphicsQueue;
} VulkanDevice;

typedef struct VulkanInfo {
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VulkanDevice device;
    u32 deltaTime;
    VkDebugUtilsMessengerEXT debugMessenger;
    // DinoArray
    const char** validationLayers;
    // VulkanDevice device;
    VkSurfaceKHR surface;
    // VulkanSwapchain swapchain;
    u32 framebufferWidth;
    u32 framebufferHeight;

    b8 recreatingSwapchain;
    u32 framebufferSizeGen;
    u32 framebufferSizeGenLast;
} VulkanInfo;

