#include "device.h"
#include "core/systems/logger.h"
#include "helpers/dinoarray.h"
#include "renderer/vulkan/vulkanTypes.h"
#include "vulkan/vulkan_core.h"

u32 isDeviceSuitable(VkPhysicalDevice pd) {
    // Returning zero means not suitable
    VkPhysicalDeviceProperties deviceProperties;
    u32 score = 0;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(pd, &deviceProperties);
    vkGetPhysicalDeviceFeatures(pd, &deviceFeatures);

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 100;
    }

    score += deviceProperties.limits.maxImageDimension2D;

    if (!deviceFeatures.geometryShader) {
        return 0;
    }
    return score;
}

b8 findQueueFamilies(VulkanInfo* vi){
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vi->device.physicalDevice, &queueFamilyCount, 0);

    VkQueueFamilyProperties* queueFamilies = dinoCreateReserveWithLengthSet(queueFamilyCount, VkQueueFamilyProperties);
    vkGetPhysicalDeviceQueueFamilyProperties(vi->device.physicalDevice, &queueFamilyCount, queueFamilies);

    for (u32 i = 0;i < queueFamilyCount;i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            vi->device.graphicsFamily = i;
        }
    }

    if (!vi->device.graphicsFamily){
        return false;
    }

    return true;
}

b8 createLogicalDevice(VulkanInfo* vi){
    VkDeviceQueueCreateInfo queueCI;
    queueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCI.queueFamilyIndex = vi->device.graphicsFamily;
    queueCI.queueCount = 1;
    float queuePriority = 1.0f;
    queueCI.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures devFeatures;

    VkDeviceCreateInfo devCI;
    devCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devCI.pQueueCreateInfos = &queueCI;
    devCI.queueCreateInfoCount = 1;

    devCI.pEnabledFeatures = &devFeatures;

    devCI.enabledExtensionCount = 0;
    devCI.enabledLayerCount = 0;

#ifdef _DEBUG
    devCI.enabledLayerCount = dinoLength(vi->validationLayers);
    devCI.ppEnabledLayerNames = vi->validationLayers;
#endif

    if (!vkCreateDevice(vi->device.physicalDevice, &devCI, vi->allocator, &vi->device.device)){
        FERROR("Failed to create device");
        return false;
    }

    vkGetDeviceQueue(vi->device.device, vi->device.graphicsFamily, 0, &vi->device.graphicsQueue);
    return true;
}

b8 pickPhysicalDevice(VulkanInfo* vi) {
    u32 deviceCnt = 0;
    vkEnumeratePhysicalDevices(vi->instance, &deviceCnt, 0);

    VkPhysicalDevice* physicalDevices =
        dinoCreateReserveWithLengthSet(deviceCnt, VkPhysicalDevice);
    vkEnumeratePhysicalDevices(vi->instance, &deviceCnt, physicalDevices);

    u32 topScore = 0;
    for (u32 i = 0; i < dinoLength(physicalDevices); i++) {
        u32 score = isDeviceSuitable(physicalDevices[i]);
        if (score > topScore) {
            topScore = score;
            vi->device.physicalDevice = physicalDevices[i];
        }
    }

    if (!findQueueFamilies(vi)){
        FERROR("Couldn't find all queue families");
        return false;
    }

    if (!createLogicalDevice(vi)){
        FERROR("Can't create logical device.");
        return false;
    }

    return true;
}
