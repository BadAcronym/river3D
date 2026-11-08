#include "river3D_main.h"
#include "river3D_vulkan.h"
#include "rv_print_macros.h"

#include "pd_dyn_arr.h"

#include "vulkan/vulkan_core.h"

#define DEVICE_EXTENSION_COUNT 1

const char *requiredExtensions[DEVICE_EXTENSION_COUNT] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

f_internal VkBool32 checkDeviceExtensionSupport
(
    VkPhysicalDevice device
){
    u32 availExtensionCount = 0;

    vkEnumerateDeviceExtensionProperties(device, 0, &availExtensionCount, 0);
    VkExtensionProperties availableExtensions[availExtensionCount];
    vkEnumerateDeviceExtensionProperties(device, 0, &availExtensionCount,
                                         availableExtensions);

    for(u16 i = 0; i < DEVICE_EXTENSION_COUNT; ++i)
    {
        StringView required = pdCstrSV(requiredExtensions[i]);

        for(u16 j = 0; j < availExtensionCount; ++j)
        {
            StringView avail = pdCstrSV(availableExtensions[j].extensionName);
            if(pdSVSame(required, avail))
            {
                goto found;
            }
        }
        RV_ERROR("unable to find extension: '"PRI_SV"'.", ARG_SV(required));
        goto notfound;

    found:
        continue;
    }

    return VK_SUCCESS;

notfound:
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

QueueFamilyIndices findQueueFamilies
(
    const VkPhysicalDevice *device,
    const VkSurfaceKHR     *surface
){
    QueueFamilyIndices indices = {0};
    u32 queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, 0);

    VkQueueFamilyProperties physicalQueueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount,
                                             physicalQueueFamilies);

    VkBool32 presentSupport = false;

    for(u32 i = 0; i < queueFamilyCount; ++i)
    {
        if(physicalQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsIndex = i;
        }

        if(physicalQueueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
           i != indices.graphicsIndex
        ){
            indices.transferIndex = i;
        }

        vkGetPhysicalDeviceSurfaceSupportKHR(*device, i, *surface, &presentSupport);
        if(presentSupport)
        {
            indices.presentIndex = i;
        }

        if( indices.graphicsIndex != UINT32_MAX &&
            indices.transferIndex != UINT32_MAX &&
            indices.presentIndex  != UINT32_MAX
        ){
            break;
        }
    }

    if(indices.transferIndex == UINT32_MAX)
    {
        indices.transferIndex = indices.graphicsIndex;
    }

    return indices;
}

f_internal u32 rateDeviceSuitability
(
    const EngineData       *engine,
    const VkPhysicalDevice *device
){
    u32 score = 0;
    QueueFamilyIndices indices = findQueueFamilies(device, &engine->vkData->surface);

    if( indices.graphicsIndex == UINT32_MAX ||
        indices.transferIndex == UINT32_MAX ||
        indices.presentIndex  == UINT32_MAX
    ){
        return 0;
    }

    VkPhysicalDeviceProperties toRateDeviceProperties = {0};
    vkGetPhysicalDeviceProperties(*device, &toRateDeviceProperties);

    VkPhysicalDeviceFeatures toRateDeviceFeatures = {0};
    vkGetPhysicalDeviceFeatures(*device, &toRateDeviceFeatures);

    if(!toRateDeviceFeatures.samplerAnisotropy ||
       !toRateDeviceFeatures.geometryShader)
    {
        return 0;
    }

    bool extensionsSupported = checkDeviceExtensionSupport(*device);
    if(!extensionsSupported)
    {
        return 0;
    }
    else
    {
        SwapchainSupportDetails swapchainSupport =
            querySwapchainSupport(device, engine->vkData->surface);
        if(!pdArrSize(swapchainSupport.formats) ||
            !pdArrSize(swapchainSupport.presentModes)
        ){
            return 0;
        }
    }

    if(toRateDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }
    score += toRateDeviceProperties.limits.maxImageDimension1D;
    score += toRateDeviceProperties.limits.maxImageDimension2D/10;
    score += toRateDeviceProperties.limits.maxImageDimension3D/100;

    if(indices.presentIndex == indices.graphicsIndex)
    {
        score += 500;
    }

    if(indices.transferIndex != indices.graphicsIndex)
    {
        score += 250;
    }

    return score;
}

SwapchainSupportDetails querySwapchainSupport
(
    const VkPhysicalDevice *physicalDevice,
    const VkSurfaceKHR     *surface
){
    SwapchainSupportDetails details = {0};
    u32                formatCount = 0;
    u32                presentModeCount = 0;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, 0);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, 0);

    if(0 != formatCount)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
                                             surface,
                                             &formatCount,
                                             details.formats.data());
    }

    if(0 != presentModeCount)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
                                                  surface,
                                                  &presentModeCount,
                                                  details.presentModes.data());
    }

    return details;
}

void pickPhysicalDevice
(
    EngineData *engine
){
    u32 deviceCount = 0;

    vkEnumeratePhysicalDevices(engine->vkData->instance, &deviceCount, 0);
    if(0 == deviceCount)
    {
        RV_ERROR("failed to find any GPU with vulkan support!");
    }

    VkPhysicalDevice devices = 0;
    pdArrReserve(devices, deviceCount);
    std::multimap<u32, VkPhysicalDevice> suitabilityCandidates;

    vkEnumeratePhysicalDevices(engine.instance, &deviceCount, devices.data());

    for(const auto& device : devices)
    {
        u32 score = rateDeviceSuitability(engine, device);
        suitabilityCandidates.insert(std::make_pair(score, device));
    }

    if(suitabilityCandidates.rbegin()->first > 0)
    {
        engine.physicalDevice = suitabilityCandidates.rbegin()->second;
    }
    else
    {
        riverLog("failed to find a suitable GPU!", RIV_LOG_LEVEL_ASSERT);
    }
}

void createLogicalDevice
(
    EngineData *engine
){
    QueueFamilyIndices indices = findQueueFamilies(engine->vkData->physicalDevice,
                                                   engine->vkData->surface);
    engine->vkData->logicalQueueFamilies.graphicsIndex = indices.graphicsIndex;
    engine->vkData->logicalQueueFamilies.transferIndex = indices.transferIndex;
    engine->vkData->logicalQueueFamilies.presentIndex  = indices.presentIndex;
    engine->vkData->msaaSamples                        = getMaxMSAASamples(engine);

    VkDeviceQueueCreateInfo queueCreateInfos = 0;
    std::set<u32> uniqueQueueFamilies =
    {
        engine->vkData->logicalQueueFamilies.graphicsIndex,
        engine->vkData->logicalQueueFamilies.transferIndex,
        engine->vkData->logicalQueueFamilies.presentIndex
    };

    float queuePriority = 1.0f;
    for(u32 queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {0};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        pdArrPush(queueCreateInfos, queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {0};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {0};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos       = queueCreateInfos;
    createInfo.queueCreateInfoCount    = pdArrSize(queueCreateInfos);
    createInfo.pEnabledFeatures        = &deviceFeatures;
    createInfo.enabledExtensionCount   = pdArrSize(deviceExtensions);
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    #ifdef DEBUG
        createInfo.enabledLayerCount   = alidationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();
    #else
        createInfo.enabledLayerCount = 0;
    #endif

    VkResult result = vkCreateDevice(engine->vkData->physicalDevice, &createInfo, 0,
                                     &engine->vkData->logicalDevice);
    RIV_ASSERT_VK_SUCCESS(result, "failed to create logical device.");

    vkGetDeviceQueue(engine->vkData->logicalDevice,
                     engine->vkData->logicalQueueFamilies.graphicsIndex, 0,
                     &engine->vkData->graphicsQueue);
    vkGetDeviceQueue(engine->vkData->logicalDevice,
                     engine->vkData->logicalQueueFamilies.transferIndex, 0,
                     &engine->vkData->transferQueue);
    vkGetDeviceQueue(engine->vkData->logicalDevice,
                     engine->vkData->logicalQueueFamilies.presentIndex, 0,
                     &engine->vkData->presentQueue);

    vkGetPhysicalDeviceProperties(engine.physicalDevice, &engine.deviceProperties);
    vkGetPhysicalDeviceFeatures(engine.physicalDevice, &deviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(engine.physicalDevice,
                                        &engine.deviceMemoryProperties);
}

VkSampleCountFlagBits getMaxMSAASamples
(
    const EngineData &engine
){
    VkSampleCountFlags counts = engine.deviceProperties.limits.framebufferColorSampleCounts &
                                engine.deviceProperties.limits.framebufferDepthSampleCounts;

    if(counts & VK_SAMPLE_COUNT_64_BIT){ return VK_SAMPLE_COUNT_64_BIT; }
    if(counts & VK_SAMPLE_COUNT_32_BIT){ return VK_SAMPLE_COUNT_32_BIT; }
    if(counts & VK_SAMPLE_COUNT_16_BIT){ return VK_SAMPLE_COUNT_16_BIT; }
    if(counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if(counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if(counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}
