#include "river3D_main.h"
#include "river3D_vulkan.h"
#include "rv_print_macros.h"

#include "vulkan/vulkan_core.h"

#ifdef DEBUG

s_global VkResult codes[61] =
{
    VK_SUCCESS,
    VK_NOT_READY,
    VK_TIMEOUT,
    VK_EVENT_SET,
    VK_EVENT_RESET,
    VK_INCOMPLETE,
    VK_ERROR_OUT_OF_HOST_MEMORY,
    VK_ERROR_OUT_OF_DEVICE_MEMORY,
    VK_ERROR_INITIALIZATION_FAILED,
    VK_ERROR_DEVICE_LOST,
    VK_ERROR_MEMORY_MAP_FAILED,
    VK_ERROR_LAYER_NOT_PRESENT,
    VK_ERROR_EXTENSION_NOT_PRESENT,
    VK_ERROR_FEATURE_NOT_PRESENT,
    VK_ERROR_INCOMPATIBLE_DRIVER,
    VK_ERROR_TOO_MANY_OBJECTS,
    VK_ERROR_FORMAT_NOT_SUPPORTED,
    VK_ERROR_FRAGMENTED_POOL,
    VK_ERROR_UNKNOWN,
    VK_ERROR_VALIDATION_FAILED,
    VK_ERROR_OUT_OF_POOL_MEMORY,
    VK_ERROR_INVALID_EXTERNAL_HANDLE,
    VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
    VK_ERROR_FRAGMENTATION,
    VK_PIPELINE_COMPILE_REQUIRED,
    VK_ERROR_NOT_PERMITTED,
    VK_ERROR_SURFACE_LOST_KHR,
    VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,
    VK_SUBOPTIMAL_KHR,
    VK_ERROR_OUT_OF_DATE_KHR,
    VK_ERROR_INCOMPATIBLE_DISPLAY_KHR,
    VK_ERROR_INVALID_SHADER_NV,
    VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR,
    VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR,
    VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR,
    VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR,
    VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR,
    VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR,
    VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT,
    VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT,
    VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT,
    VK_THREAD_IDLE_KHR,
    VK_THREAD_DONE_KHR,
    VK_OPERATION_DEFERRED_KHR,
    VK_OPERATION_NOT_DEFERRED_KHR,
    VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR,
    VK_ERROR_COMPRESSION_EXHAUSTED_EXT,
    VK_INCOMPATIBLE_SHADER_BINARY_EXT,
    VK_PIPELINE_BINARY_MISSING_KHR,
    VK_ERROR_NOT_ENOUGH_SPACE_KHR,
    VK_ERROR_VALIDATION_FAILED_EXT,
    VK_ERROR_OUT_OF_POOL_MEMORY_KHR,
    VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR,
    VK_ERROR_FRAGMENTATION_EXT,
    VK_ERROR_NOT_PERMITTED_EXT,
    VK_ERROR_NOT_PERMITTED_KHR,
    VK_ERROR_INVALID_DEVICE_ADDRESS_EXT,
    VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR,
    VK_PIPELINE_COMPILE_REQUIRED_EXT,
    VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT,
    VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT // is a legacy alias
};

s_global StringView codeStrings[62] =
{
    { .data = "VK_SUCCESS",                                            .size = 10 },
    { .data = "VK_NOT_READY",                                          .size = 12 },
    { .data = "VK_TIMEOUT",                                            .size = 10 },
    { .data = "VK_EVENT_SET",                                          .size = 12 },
    { .data = "VK_EVENT_RESET",                                        .size = 14 },
    { .data = "VK_INCOMPLETE",                                         .size = 13 },
    { .data = "VK_ERROR_OUT_OF_HOST_MEMORY",                           .size = 27 },
    { .data = "VK_ERROR_OUT_OF_DEVICE_MEMORY",                         .size = 29 },
    { .data = "VK_ERROR_INITIALIZATION_FAILED",                        .size = 30 },
    { .data = "VK_ERROR_DEVICE_LOST",                                  .size = 20 },
    { .data = "VK_ERROR_MEMORY_MAP_FAILED",                            .size = 26 },
    { .data = "VK_ERROR_LAYER_NOT_PRESENT",                            .size = 26 },
    { .data = "VK_ERROR_EXTENSION_NOT_PRESENT",                        .size = 30 },
    { .data = "VK_ERROR_FEATURE_NOT_PRESENT",                          .size = 28 },
    { .data = "VK_ERROR_INCOMPATIBLE_DRIVER",                          .size = 28 },
    { .data = "VK_ERROR_TOO_MANY_OBJECTS",                             .size = 25 },
    { .data = "VK_ERROR_FORMAT_NOT_SUPPORTED",                         .size = 29 },
    { .data = "VK_ERROR_FRAGMENTED_POOL",                              .size = 24 },
    { .data = "VK_ERROR_UNKNOWN",                                      .size = 16 },
    { .data = "VK_ERROR_VALIDATION_FAILED",                            .size = 26 },
    { .data = "VK_ERROR_OUT_OF_POOL_MEMORY",                           .size = 27 },
    { .data = "VK_ERROR_INVALID_EXTERNAL_HANDLE",                      .size = 32 },
    { .data = "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS",               .size = 39 },
    { .data = "VK_ERROR_FRAGMENTATION",                                .size = 22 },
    { .data = "VK_PIPELINE_COMPILE_REQUIRED",                          .size = 28 },
    { .data = "VK_ERROR_NOT_PERMITTED",                                .size = 22 },
    { .data = "VK_ERROR_SURFACE_LOST_KHR",                             .size = 25 },
    { .data = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR",                     .size = 34 },
    { .data = "VK_SUBOPTIMAL_KHR",                                     .size = 17 },
    { .data = "VK_ERROR_OUT_OF_DATE_KHR",                              .size = 24 },
    { .data = "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR",                     .size = 34 },
    { .data = "VK_ERROR_INVALID_SHADER_NV",                            .size = 26 },
    { .data = "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR",                .size = 38 },
    { .data = "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR",       .size = 47 },
    { .data = "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR",    .size = 50 },
    { .data = "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR",       .size = 47 },
    { .data = "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR",        .size = 46 },
    { .data = "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR",          .size = 44 },
    { .data = "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT", .size = 53 },
    { .data = "VK_ERROR_PRESENT_TIMING_QUEUE_FULL_EXT",                .size = 38 },
    { .data = "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT",          .size = 44 },
    { .data = "VK_THREAD_IDLE_KHR",                                    .size = 18 },
    { .data = "VK_THREAD_DONE_KHR",                                    .size = 18 },
    { .data = "VK_OPERATION_DEFERRED_KHR",                             .size = 25 },
    { .data = "VK_OPERATION_NOT_DEFERRED_KHR",                         .size = 29 },
    { .data = "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR",             .size = 41 },
    { .data = "VK_ERROR_COMPRESSION_EXHAUSTED_EXT",                    .size = 34 },
    { .data = "VK_INCOMPATIBLE_SHADER_BINARY_EXT",                     .size = 33 },
    { .data = "VK_PIPELINE_BINARY_MISSING_KHR",                        .size = 30 },
    { .data = "VK_ERROR_NOT_ENOUGH_SPACE_KHR",                         .size = 29 },
    { .data = "VK_ERROR_VALIDATION_FAILED_EXT",                        .size = 30 },
    { .data = "VK_ERROR_OUT_OF_POOL_MEMORY_KHR",                       .size = 31 },
    { .data = "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR",                  .size = 36 },
    { .data = "VK_ERROR_FRAGMENTATION_EXT",                            .size = 26 },
    { .data = "VK_ERROR_NOT_PERMITTED_EXT",                            .size = 26 },
    { .data = "VK_ERROR_NOT_PERMITTED_KHR",                            .size = 26 },
    { .data = "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT",                   .size = 35 },
    { .data = "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR",           .size = 43 },
    { .data = "VK_PIPELINE_COMPILE_REQUIRED_EXT",                      .size = 32 },
    { .data = "VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT",                .size = 38 },
    { .data = "VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT",               .size = 39 },
    { .data = "RV_VK_ERROR_UNKNOWN",                                   .size = 19 }
};

StringView rvTranslateVkResult
(
    const VkResult code
){
    for(u08 i = 0; i < 61; ++i)
    {
        if(code == codes[i])
        {
            return codeStrings[i];
        }
    }

    return codeStrings[61];
}

f_internal VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT  *callbackData,
    void                                        *userData
){
    //can I know which layer is outputting the msg?
    RV_DEBUG("VL says: %s with messageType 0x%X.",
             callbackData->pMessage, messageType);

    return VK_FALSE;
}

f_internal VkResult CreateDebugUtilsMessengerEXT
(
    const EngineData                         *engine,
    const VkDebugUtilsMessengerCreateInfoEXT *createInfo,
    const VkAllocationCallbacks              *allocator
){
    vkCreateMsgr func = (vkCreateMsgr)
                        vkGetInstanceProcAddr(engine->vkData->instance,
                                              "vkCreateDebugUtilsMessengerEXT");
    if(func)
    {
        return func(engine->vkData->instance, createInfo, allocator,
                    &engine->vkData->debugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

f_internal void DestroyDebugUtilsMessengerEXT
(
    const EngineData            *engine,
    const VkAllocationCallbacks *allocator
){
    vkDestroyMsgr func = (vkDestroyMsgr)
                         vkGetInstanceProcAddr(engine->vkData->instance,
                                               "vkDestroyDebugUtilsMessengerEXT");
    if(func)
    {
        func(engine->vkData->instance, engine->vkData->debugMessenger, allocator);
    }
}

f_internal bool checkValidationLayerSupport
(
    void
){
    u32 availLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&availLayerCount, 0);

    VkLayerProperties layers[availLayerCount];
    vkEnumerateInstanceLayerProperties(&availLayerCount, layers);

    for(u16 i = 0; i < VALIDATION_LAYER_COUNT; ++i)
    {
        StringView required = pdCstrSV(validationLayers[i]);

        for(u16 j = 0; j < availLayerCount; ++j)
        {
            StringView avail = pdCstrSV(layers[j].layerName);
            if(pdSVSame(required, avail))
            {
                goto found;
            }
        }
        RV_ERROR("unable to find validation layer: '"PRI_SV"'.", ARG_SV(required));
        goto notfound;

    found:
        continue;
    }

    return true;

notfound:
    return false;
}

f_internal void populateDebugMessengerCreateInfo
(
    VkDebugUtilsMessengerCreateInfoEXT *createInfo
){
    *createInfo = (VkDebugUtilsMessengerCreateInfoEXT){0};
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
}

f_internal void setupDebugMessenger
(
    EngineData *engine
){

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
    populateDebugMessengerCreateInfo(&createInfo);

    VkResult result = CreateDebugUtilsMessengerEXT(engine, &createInfo, 0);
    RV_ASSERT(result == VK_SUCCESS, "failed to set up debug messenger.");
}

#endif

f_internal StringView* getRequiredExtensions
(
    void
){
    // NOTE: need a replacement for GLFW here D:
    u32 glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    #ifdef DEBUG
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    #endif

    return extensions;
}

f_internal bool checkInstanceExtensions
(
    StringView            *requiredExt,
    VkExtensionProperties *instanceExt
){
    for(const auto &required : *requiredExt)
    {
        VkBool32 extFound = VK_FALSE;

            for(const auto &present : *instanceExt)
            {
                if(0 == strcmp(required, present.extensionName))
                {
                    extFound = VK_TRUE;
                    break;
                }
            }
        if(!extFound)
        {
            StringView msg = cstr_sv("extension not found: ");
            msg += required;

            riverLog(msg, RIV_LOG_LEVEL_ERROR);

            return VK_FALSE;
        }
    }
    return VK_TRUE;
}

f_internal void createInstance
(
    EngineData *engine
){
    RV_ASSERT(checkValidationLayerSupport(),
              "validation layers requested, but not available!");

    char cstrBuf[256] = {0};

    VkApplicationInfo appInfo = {0};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = pdSVCstr(engine->config.projectName, cstrBuf);
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 1);
    appInfo.pEngineName        = ENGINE_NAME;
    appInfo.engineVersion      = VK_MAKE_API_VERSION(0, 0, 0, 1);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    u32 instanceExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(0, &instanceExtensionCount, 0);
    VkExtensionProperties instanceExtensions[instanceExtensionCount];
    vkEnumerateInstanceExtensionProperties(0, &instanceExtensionCount,
                                           instanceExtensions);

    StringView *requiredExtensions = getRequiredExtensions();

    bool extOK = checkInstanceExtensions(&requiredExtensions, &instanceExtensions);
    RV_ASSERT(extOK, "extensions required, but not available!");

    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledExtensionCount   = requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions;
    createInfo.enabledLayerCount       = 0;

    #ifdef DEBUG
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};
        createInfo.enabledLayerCount   = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers;

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    #else
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = 0;
    #endif

    VkResult result = vkCreateInstance(&createInfo, 0, &engine->vkData->instance);
    RV_ASSERT(result == VK_SUCCESS, "failed to create instance.");
}

void vkInit
(
    EngineData *engine
){
    createInstance(engine);

    #ifdef DEBUG
        setupDebugMessenger(engine);
    #endif

    createSurface(engine);
    pickPhysicalDevice(engine);
    createLogicalDevice(engine);

    createSwapchain(engine, settings);

    createRenderPass(engine);
    createDescriptorSetLayout(engine);
    createGraphicsPipeline(engine, manifest);

    createCommandPools(engine);
    createDepthResources(engine);
    createFramebuffers(engine);

    createTextureImage(engine, manifest);
    createTextureSampler(engine);

    loadModel(engine, manifest.projectModelPath);
    createVertexBuffer(engine);
    createUniformBuffers(engine);

    createDescriptorPool(engine);
    createDescriptorSets(engine);

    createCommandBuffers(engine);
    createSyncObjects(engine);
}

void vkShutdown
(
    EngineData *engine
){
    vkDeviceWaitIdle(engine->vkData->logicalDevice);

    cleanupSyncObjects(engine);
    cleanupSwapchain(engine);

    vkDestroySampler(engine->vkData->logicalDevice, engine->vkData->textureSampler, 0);

    vkDestroyImage(engine->vkData->logicalDevice, engine->vkData->textureImage, 0);
    vkFreeMemory(engine->vkData->logicalDevice, engine->vkData->textureImageMemory, 0);
    vkDestroyImageView(engine->vkData->logicalDevice,
                       engine->vkData->textureImageView, 0);

    vkDestroyBuffer(engine->vkData->logicalDevice, engine->vkData->vertexBuffer, 0);
    vkFreeMemory(engine->vkData->logicalDevice, engine->vkData->vertexBufferMemory, 0);

    vkDestroyDescriptorPool(engine->vkData->logicalDevice,
                            engine->vkData->descriptorPool, 0);
    vkDestroyDescriptorSetLayout(engine->vkData->logicalDevice,
                                 engine->vkData->descriptorSetLayout, 0);

    vkDestroyPipeline(engine->vkData->logicalDevice,
                      engine->vkData->graphicsPipeline, 0);
    vkDestroyPipelineLayout(engine->vkData->logicalDevice,
                            engine->vkData->graphicsPipelineLayout, 0);

    vkDestroyRenderPass(engine->vkData->logicalDevice, engine->vkData->renderPass, 0);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroyBuffer(engine->vkData->logicalDevice,
                        engine->vkData->uniformBuffers[i], 0);
        vkFreeMemory(engine->vkData->logicalDevice,
                     engine->vkData->uniformBuffersMemory[i], 0);
    }

    vkDestroyCommandPool(engine->vkData->logicalDevice,
                         engine->vkData->graphicsCommandPool, 0);
    vkDestroyCommandPool(engine->vkData->logicalDevice,
                         engine->vkData->transferCommandPool, 0);
    vkDestroyDevice(engine->vkData->logicalDevice, 0);

    #ifdef DEBUG
        DestroyDebugUtilsMessengerEXT(engine, 0);
    #endif

    vkDestroySurfaceKHR(engine->vkData->instance, engine->vkData->surface, 0);
    vkDestroyInstance(engine->vkData->instance, 0);
}

void drawFrame
(
    EngineData *engine
){
    vkWaitForFences(engine->vkData->logicalDevice,
                    1, &engine->vkData->inFlightFences[currentFrame],
                    VK_TRUE, UINT64_MAX);

    u32 imageIndex = 0;

    VkResult result = vkAcquireNextImageKHR(engine->vkData->logicalDevice,
                                            engine->vkData->swapchain,
                                            UINT64_MAX,
                                            engine->vkData->acquireSemaphore,
                                            VK_NULL_HANDLE, &imageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain(engine, settings);
        return;
    }
    else if(result != VK_SUBOPTIMAL_KHR)
    {
        RV_ASSERT(result == VK_SUCCESS, "failed to acquire swapchain image!");
    }

    vkResetFences(engine->vkData->logicalDevice, 1,
                  &engine->vkData->inFlightFences[currentFrame]);

    updateUniformBuffer(engine, currentFrame);

    vkResetCommandBuffer(engine->vkData->commandBuffers[currentFrame], 0);
    recordCommandBuffer(engine, engine->vkData->commandBuffers[currentFrame],
                        imageIndex);

    std::array<VkSemaphore, 1> waitSemaphores =
    {
        engine.acquireSemaphore
    };

    VkSemaphore signalSemaphores[1] =
    {
        engine.imageReadyForPresentSemaphores[imageIndex]
    };

    VkPipelineStageFlags waitStages[2] =
    {
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSubmitInfo drawSubmitInfo = {0};
    drawSubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    drawSubmitInfo.waitSemaphoreCount   = (u32)(pdArrSize(waitSemaphores));
    drawSubmitInfo.pWaitSemaphores      = waitSemaphores;
    drawSubmitInfo.signalSemaphoreCount = (u32)(pdArrSize(signalSemaphores));
    drawSubmitInfo.pSignalSemaphores    = signalSemaphores;
    drawSubmitInfo.pWaitDstStageMask    = waitStages;
    drawSubmitInfo.commandBufferCount   = 1;
    drawSubmitInfo.pCommandBuffers      = &engine->vkData->commandBuffers[currentFrame];

    result = vkQueueSubmit(engine->vkData->graphicsQueue,
                           1, &drawSubmitInfo,
                           engine->vkData->inFlightFences[currentFrame]);
    RV_ASSERT(result == VK_SUCCESS, "failed to submit draw command buffer!");

    VkSwapchainKHR swapchains[1] =
    {
        engine->vkData->swapchain
    };

    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &engine.imageReadyForPresentSemaphores[imageIndex];
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = swapchains;
    presentInfo.pImageIndices      = &imageIndex;
    presentInfo.pResults           = 0;

    VkSemaphore tmp = engine->vkData->iRWSemaphores[imageIndex];
    engine->vkData->iRWSemaphores[imageIndex] = engine->vkData->acquireSemaphore;
    engine->vkData->acquireSemaphore = tmp;

    result = vkQueuePresentKHR(engine->vkData->presentQueue, &presentInfo);

    if(result == VK_ERROR_OUT_OF_DATE_KHR ||
       result == VK_SUBOPTIMAL_KHR        ||
       engine->framebufferResized
    ){
        engine->framebufferResized = false;
        recreateSwapchain(engine, settings);
    }
    else
    {
        RV_ASSERT(result == VK_SUCCESS, "failed to present swapchain image!");
    }

    currentFrame = (++currentFrame) % MAX_FRAMES_IN_FLIGHT;
}
