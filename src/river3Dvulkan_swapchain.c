#include "river3D_main.h"
#include "rv_print_macros.h"

#include "pd_dyn_arr.h"

#include "vulkan/vulkan_core.h"

f_internal VkExtent2D chooseSwapExtent
(
    const VkSurfaceCapabilitiesKHR *capabilities,
    GLFWwindow                     *window
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

        actualExtent.width = clamp(actualExtent.width,
                                   capabilities->minImageExtent.width,
                                   capabilities->maxImageExtent.width);

        actualExtent.height = clamp(actualExtent.height,
                                    capabilities->minImageExtent.height,
                                    capabilities->maxImageExtent.height);

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
        if(curr == engine->vkData->presentMode)
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
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(engine->vkData->logicalDevice,
                                           &createInfo, 0, &engine->vkData->swapchain);
    RV_ASSERT(result == VK_SUCCESS, "failed to create swapchain!");

    vkGetSwapchainImagesKHR(engine->vkData->logicalDevice, engine->vkData->swapchain,
                            &engine->vkData->swapchainImageCount, 0);

    pdArrReserve(engine->vkData->swapchainImages, engine->vkData->swapchainImageCount);

    vkGetSwapchainImagesKHR(engine->vkData->logicalDevice, engine->vkData->swapchain,
                            &engine->vkData->swapchainImageCount,
                            engine->vkData->swapchainImages);

    engine->vkData->swapchainImageFormat = surfaceFormat.format;
    engine->vkData->swapchainExtent      = extent;

    pdArrReserve(engine->vkData->swapchainImageViews,
                 pdArrSize(engine->vkData->swapchainImages));

    for(u32 i = 0; i < pdArrSize(engine->vkData->swapchainImages); ++i)
    {
        engine->vkData->swapchainImageViews[i] =
            createImageView(engine, engine->vkData->swapchainImages[i],
                            1, engine->vkData->swapchainImageFormat,
                            VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void createRenderPass
(
    EngineData *engine
){
    VkAttachmentDescription colorAttachmentDescription = {0};
    colorAttachmentDescription.format         = engine->vkData->swapchainImageFormat;
    colorAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {0};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkFormat candidates[3] =
    {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    VkAttachmentDescription depthAD = {0};
    depthAD.format =
        findSupportedFormat(engine, candidates,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    depthAD.samples        = VK_SAMPLE_COUNT_1_BIT;
    depthAD.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAD.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAD.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAD.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAD.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAD.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {0};
    depthAttachmentRef.attachment= 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                               VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                               VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[2] =
    {
        colorAttachmentDescription,
        depthAttachmentDescription
    };

    VkRenderPassCreateInfo renderPassCreateInfo = {0};
    renderPassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 2;
    renderPassCreateInfo.pAttachments    = attachments;
    renderPassCreateInfo.subpassCount    = 1;
    renderPassCreateInfo.pSubpasses      = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies   = &dependency;

    VkResult result = vkCreateRenderPass(engine->vkData->logicalDevice,
                                         &renderPassCreateInfo, 0,
                                         &engine->vkData->renderPass);
    RV_ASSERT(result == VK_SUCCESS, "failed to create render pass!");
}

void cleanupSwapchain
(
    const EngineData *engine
){
    vkDestroyImageView(engine.logicalDevice, engine.depthImageView, 0);
    vkDestroyImage(engine.logicalDevice, engine.depthImage, 0);
    vkFreeMemory(engine.logicalDevice, engine.depthImageMemory, 0);

    for(size_t i = 0; i < pdArrSize(engine->vkData->swapchainFramebuffers); ++i)
    {
        vkDestroyFramebuffer(engine->vkData->logicalDevice,
                             engine->vkData->swapchainFramebuffers[i], 0);
    }

    for(size_t i = 0; i < pdArrSize(engine->vkData->swapchainImageViews); ++i)
    {
        vkDestroyImageView(engine->vkData->logicalDevice,
                           engine->vkData->swapchainImageViews[i], 0);
    }

    vkDestroySwapchainKHR(engine->vkData->logicalDevice, engine->vkData->swapchain, 0);
}

void recreateSwapchain
(
    EngineData         *engine,
    const UserSettings *settings
){
    i32 width  = 0;
    i32 height = 0;

    glfwGetFramebufferSize(engine->window, &width, &height);

    while(width == 0 || height == 0)
    {
        glfwGetFramebufferSize(engine->window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(engine->vkData->logicalDevice);

    cleanupSwapchain(engine);

    createSwapchain(engine, settings);
    createDepthResources(engine);
    createFramebuffers(engine);

    RV_TRACE("recreated swapchain: %"PRIi32"x%"PRIi32, width, height);
}
