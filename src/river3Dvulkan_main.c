#include "river3D_main.h"
#include "rv_print_macros.h"

#include "pd_int_types.h"

f_internal VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT     messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT            messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void                                       *pUserData
){
    RV_ERROR("validation layer: %s", pCallbackData->pMessage);
    return VK_FALSE;
}

f_internal VkResult CreateDebugUtilsMessengerEXT
(
    VkInstance                               instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks              *pAllocator,
    VkDebugUtilsMessengerEXT                 *pDebugMessenger
){
    vkCreateMsgr func = (vkCreateMsgr)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if(func)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

f_internal std::vector<const char*> getRequiredExtensions
(
    void
){
    u32 glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    #ifdef DEBUG
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    #endif

    return extensions;
}

f_internal bool checkValidationLayerSupport
(
    void
){
    u32 layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, 0);

    VkLayerProperties layers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, layers);

    for(const char *layer : device::validationLayers)
    {
        for(const auto &layerPresent : layerVec)
        {
            if(0 == strcmp(layerPresent.layerName, layer))
            {
                layerFound = true;
                break;
            }
        }
    }

    return true;
}

f_internal void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo                 = {};
    createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

f_internal bool checkInstanceExtensions
(
    std::vector<const char*> *requiredExt,
    std::vector<VkExtensionProperties> *instanceExt
){
    RV_DEBUG("Present:");
    for(const auto &extension : *instanceExt)
    {
        RV_DEBUG("%s", extension.extensionName);
    }

    RV_DEBUG("Required:");
    for(const auto &required : *requiredExt)
    {
        for(const auto &present : *instanceExt)
        {
            if(0 == strcmp(required, present.extensionName))
            {
                    RV_DEBUG("found:\t", 0, 0);
                    RV_DEBUG(required, 1, 1);
                    extFound = true;
                    break;
                }
            }
            if(!extFound)
            {
                RV_DEBUG("not found:\t", 0, 0);
                return false;
            }
        }
    }

    return true;
}

void rvCreateInstance
(
    void
){
    #ifdef DEBUG
    if(!checkValidationLayerSupport())
    {
        RV_ERROR("validation layers requested, but not available!");
    }
    #endif

    VkApplicationInfo appInfo = {0};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = river::appName;
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.pEngineName        = river::engineName;
    appInfo.engineVersion      = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    u32 instanceExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(0, &instanceExtensionCount, 0);
    VkExtensionProperties instanceExtensions[instanceExtensionCount];
    vkEnumerateInstanceExtensionProperties(0, &instanceExtensionCount,
                                           instanceExtensions);

    std::vector<const char*> requiredExtensions = getRequiredExtensions();
    if(checkInstanceExtensions(&requiredExtensions, &instanceExtensions)){
        RV_DEBUG("\nAll required extensions are present.", 0, 1);
    }else{
        RV_DEBUG("\nERROR: extensions required, but not available!", 2, 1);
        throw std::runtime_error("extensions required, but not available!");
    }

    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledExtensionCount   = static_cast<u32>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledLayerCount       = 0;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if(build_DEBUG){
        createInfo.enabledLayerCount = static_cast<u32>(device::validationLayers.size());
        createInfo.ppEnabledLayerNames = device::validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }else{
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = 0;
    }

    if(vkCreateInstance(&createInfo, 0, &instance) != VK_SUCCESS){
        RV_DEBUG("\nERROR: failed to create instance.", 2, 1);
        throw std::runtime_error("failed to create instance.");
    }else{
        RV_DEBUG("Successfully created instance.", 0, 1);
    }
}

void rvSetupDebugMessenger
(
    void
){
    #ifndef DEBUG
        return;
    #endif

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
    populateDebugMessengerCreateInfo(createInfo);

    if(CreateDebugUtilsMessengerEXT(instance, &createInfo, 0, &device::debugMessenger) != VK_SUCCESS)
    {
        RV_ERROR("failed to set up debug messenger.");
    }
    else
    {
        RV_DEBUG("Successfully set up debug messenger.", 0, 1);
    }
}

void rvDestroyDebugUtilsMessengerEXT
(
    VkInstance                  instance,
    VkDebugUtilsMessengerEXT    messenger,
    const VkAllocationCallbacks *pAllocator
){
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func)
    {
        func(instance, messenger, pAllocator);
    }
}

void rvCreateSurface()
{
    if(glfwCreateWindowSurface(instance, window::pWindow, 0, &swap::surface) != VK_SUCCESS)
    {
        RV_ERROR("failed to create window surface!");
    }
    else
    {
        RV_DEBUG("Successfully created window surface.");
    }
}

void rvCreateFramebuffers
(
    void
){
    swapchainFramebuffers.reserve(swapchainImageViews.size());

    for(size_t i = 0; i < swapchainImageViews.size(); ++i)
    {
        VkImageView attachments[] = { swapchainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = pipeline::renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments    = attachments;
        framebufferInfo.width           = swapchainExtent.width;
        framebufferInfo.height          = swapchainExtent.height;
        framebufferInfo.layers          = 1;

        if(vkCreateFramebuffer(device::logicalDevice, &framebufferInfo, 0, &swapchainFramebuffers[i]) != VK_SUCCESS)
        {
            RV_ERROR("failed to create framebuffer!");
        }
        else
        {
            RV_DEBUG("Successfully created framebuffer.");
        }
    }
}

void rvCreateCommandPool
(
    void
){

    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if((vkCreateCommandPool(device::logicalDevice, &poolInfo, 0, &pipeline::commandPool)) != VK_SUCCESS)
    {
        RV_ERROR("failed to create command pool!");
    }
    else
    {
        RV_DEBUG("Successfully created command pool.");
    }
}

void rvCreateCommandBuffer
(
    void
){
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = pipeline::commandPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if(vkAllocateCommandBuffers(device::logicalDevice, &allocInfo, &pipeline::commandBuffer) != VK_SUCCESS)
    {
        RV_ERROR("failed to allocate command buffers!");
    }
    else
    {
        RV_DEBUG("Successfully allocated command buffer.", 0, 1);
    }
}

void rvRecordCommandBuffer
(
    VkCommandBuffer commandBuffer,
    u32             imageIndex
){
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if((vkBeginCommandBuffer(commandBuffer, &beginInfo)) != VK_SUCCESS)
    {
        RV_ERROR("failed to begin recording command buffer!");
    }

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass        = pipeline::renderPass;
    renderPassInfo.framebuffer       = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    renderPassInfo.clearValueCount   = 1;
    renderPassInfo.pClearValues      = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline::graphicsPipeline);

    VkViewport viewport {0};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = (float)(swapChainExtent.width);
    viewport.height   = (float)(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    //NOTE: I'm specifyin the num of verts... lmfao
    vkCmdDraw(commandBuffer, 3, 2, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        RV_ERROR("failed to record command buffer!");
    }
}

void rvCreateSyncObjects
(
    void
){
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if((vkCreateSemaphore(logicalDevice, &semaphoreInfo, 0, &imageAvailableSemaphore)) != VK_SUCCESS ||
       (vkCreateSemaphore(logicalDevice, &semaphoreInfo, 0, &renderFinishedSemaphore)) != VK_SUCCESS)
    {
        RV_ERROR("\nERROR: failed to create semaphores!");
    }
    else
    {
        RV_DEBUG("Successfully created semaphores.", 0, 1);
    }

    if((vkCreateFence(device::logicalDevice, &fenceInfo, 0, &pipeline::inFlightFence)) != VK_SUCCESS)
    {
        RV_ERROR("failed to create fence!");
    }
    else
    {
        RV_DEBUG("Successfully created fence.");
    }
}
