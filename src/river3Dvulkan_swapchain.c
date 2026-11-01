#include "river3D_main.h"

#include "pd_dyn_arr.h"

#include "vulkan/vulkan_core.h"

f_internal VkExtent2D chooseSwapExtent
(
    const VkSurfaceCapabilitiesKHR *capabilities
    // GLFWwindow                     *window
){
    if(capabilities->currentExtent.width != U32MAX)
    {
        return capabilities->currentExtent;
    }
    else
    {
        u32 width  = 0;
        u32 height = 0;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent =
        {
            width, height
        };

        actualExtent.width = std::clamp(actualExtent.width,
                                        capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);

        actualExtent.height = std::clamp(actualExtent.height,
                                         capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void createSwapchain
(
    EngineData *engine
){
    SwapchainSupportDetails swapchainSupport =
        querySwapchainSupport(engine->vkData->physicalDevice, engine->vkData->surface);
    VkSurfaceFormatKHR surfaceFormat = swapchainSupport.formats[0];

    u64 arrSize = pdArrSize(swapchainSupport.formats);
    for(u64 i = 0; i < arrSize; ++i)
    {
        VkSurfaceFormatKHR curr = swapchainSupport.formats[i];
        if(curr.format     == VK_FORMAT_B8G8R8A8_SRGB &&
           curr.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        ){
            surfaceFormat = curr;
        }
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; //fallback

    arrSize = pdArrSize(swapchainSupport.presentModes);
    for(u64 i = 0; i < arrSize; ++i)
    {
        VkPresentModeKHR curr = swapchainSupport.presentModes[i];
        if(curr == engine->config.presentMode)
        {
            presentMode = curr;
        }
    }

    VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities, engine->window);

    engine->vkData->swapchainImageCount =
        swapchainSupport.capabilities.minImageCount + 1;

    if(swapchainSupport.capabilities.maxImageCount > 0 &&
       engine->vkData->swapchainImageCount >
       swapchainSupport.capabilities.maxImageCount
    ){
        engine->vkData->swapchainImageCount =
            swapchainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {0};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = engine->vkData->surface;
    createInfo.minImageCount    = engine->vkData->swapchainImageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    u32 queueFamilyIndices[] =
    {
        engine->vkData->logicalQueueFamilies.graphicsIndex,
        engine->vkData->logicalQueueFamilies.presentIndex
    };

    if(engine->vkData->logicalQueueFamilies.graphicsIndex !=
       engine->vkData->logicalQueueFamilies.presentIndex)
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = sizeof(queueFamilyIndices)/sizeof(u32);
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = 0;
    }

    createInfo.preTransform   = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(engine.logicalDevice, &createInfo, 0,
                                           &engine.swapchain);
    RV_ASSERT(result == VK_SUCCESS, "failed to create swapchain!");

    vkGetSwapchainImagesKHR(engine.logicalDevice, engine.swapchain,
                            &engine.swapchainImageCount, 0);

    engine.swapchainImages.resize(engine.swapchainImageCount);

    vkGetSwapchainImagesKHR(engine.logicalDevice, engine.swapchain,
                            &engine.swapchainImageCount, engine.swapchainImages.data());

    engine.swapchainImageFormat = surfaceFormat.format;
    engine.swapchainExtent      = extent;

    engine.swapchainImageViews.resize(engine.swapchainImages.size());

    for(u32 i = 0; i < engine.swapchainImages.size(); ++i)
    {
        engine.swapchainImageViews[i] = createImageView(engine,
                                                        engine.swapchainImages[i],
                                                        1,
                                                        engine.swapchainImageFormat,
                                                        VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void createRenderPass
(
    EngineData &engine
){
    VkAttachmentDescription colorAttachmentDescription{};
    colorAttachmentDescription.format = engine.swapchainImageFormat;
    colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    const std::vector<VkFormat> candidates =
    {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    VkAttachmentDescription depthAttachmentDescription{};
    depthAttachmentDescription.format = findSupportedFormat(engine,
                                                            candidates,
                                                            VK_IMAGE_TILING_OPTIMAL,
                                                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    depthAttachmentDescription.samples  = VK_SAMPLE_COUNT_1_BIT;
    depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment= 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments =
    {
        colorAttachmentDescription,
        depthAttachmentDescription
    };

    VkRenderPassCreateInfo renderPassCreateInfo = {0};
    renderPassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = (u32)(attachments.size());
    renderPassCreateInfo.pAttachments    = attachments.data();
    renderPassCreateInfo.subpassCount    = 1;
    renderPassCreateInfo.pSubpasses      = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies   = &dependency;

    VkResult result = vkCreateRenderPass(engine.logicalDevice, &renderPassCreateInfo, 0, &engine.renderPass);
    RIV_ASSERT_VK_SUCCESS(result, "failed to create render pass!");
}

void cleanupSwapchain
(
    const EngineData *engine
){
    vkDestroyImageView(engine.logicalDevice, engine.depthImageView, 0);
    vkDestroyImage(engine.logicalDevice, engine.depthImage, 0);
    vkFreeMemory(engine.logicalDevice, engine.depthImageMemory, 0);

    for(size_t i = 0; i < engine.swapchainFramebuffers.size(); ++i)
    {
        vkDestroyFramebuffer(engine.logicalDevice, engine.swapchainFramebuffers[i], 0);
    }

    for(size_t i = 0; i < engine.swapchainImageViews.size(); ++i)
    {
        vkDestroyImageView(engine.logicalDevice, engine.swapchainImageViews[i], 0);
    }

    vkDestroySwapchainKHR(engine.logicalDevice, engine.swapchain, 0);
}

void recreateSwapchain
(
    EngineData          &engine,
    const UserSettings  &settings
){
    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(engine.window, &width, &height);

    while(width == 0 || height == 0)
    {
        glfwGetFramebufferSize(engine.window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(engine.logicalDevice);

    cleanupSwapchain(engine);

    createSwapchain(engine, settings);
    createDepthResources(engine);
    createFramebuffers(engine);

    riverLog(std::format("recreated swapchain: {}x{}", width, height), RIV_LOG_LEVEL_TRACE);
}
