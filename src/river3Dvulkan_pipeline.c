#include "vulkan/vulkan_core.h"

#include "river3D_main.h"
#include "rv_print_macros.h"

#include "pd_dyn_arr.h"

s_global VkDynamicState dynamicStates[2] =
{
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

f_internal VkShaderModule createShaderModule
(
    const EngineData *engine,
    StringView       code
){
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size;
    createInfo.pCode    = (const uint32_t*)(code.data);

    VkShaderModule shaderModule = {0};
    VkResult result = vkCreateShaderModule(engine->vkData->logicalDevice,
                                           &createInfo, 0, &shaderModule);
    RV_ASSERT(result == VK_SUCCESS, "failed to create shader module!");

    return shaderModule;
}

void createGraphicsPipeline
(
    EngineData *engine
){
    StringView vertShaderCode = pdSVReadFile("src/shaders/vertTest.vert");
    StringView fragShaderCode = pdSVReadFile("src/shaders/fragTest.frag");

    VkShaderModule vertShaderModule = createShaderModule(engine, vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(engine, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {0};
    vertShaderStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertShaderModule;
    vertShaderStageCreateInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {0};
    fragShaderStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = fragShaderModule;
    fragShaderStageCreateInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageCreateInfo, fragShaderStageCreateInfo };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {0};
    dynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = (u32)(pdArrSize(dynamicStates));
    dynamicStateCreateInfo.pDynamicStates    = dynamicStates;

    VkVertexInputBindingDescription   bindingDesc   = getVertexBindingDescription();
    VkVertexInputAttributeDescription attributeDesc = getVertexAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo pipelineVISCI = {0};
    pipelineVISCI.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineVISCI.vertexBindingDescriptionCount   = 1;
    pipelineVISCI.pVertexBindingDescriptions      = &bindingDescription;
    pipelineVISCI.vertexAttributeDescriptionCount = (u32)(pdArrSize(attributeDescriptions));
    pipelineVISCI.pVertexAttributeDescriptions    = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo pipelineIASCI = {0};
    pipelineIASCI.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineIASCI.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineIASCI.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {0};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = (float)engine->vkData->swapchainExtent.width;
    viewport.height   = (float)engine->vkData->swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {0};
    scissor.extent = engine->vkData->swapchainExtent;

    VkPipelineViewportStateCreateInfo viewportSCI = {0};
    viewportSCI.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportSCI.viewportCount = 1;
    viewportSCI.scissorCount  = 1;

    VkPipelineRasterizationStateCreateInfo rasterizerSCI = {0};
    rasterizerSCI.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerSCI.depthClampEnable        = VK_FALSE;
    rasterizerSCI.rasterizerDiscardEnable = VK_FALSE;
    rasterizerSCI.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizerSCI.lineWidth               = 1.0f;
    rasterizerSCI.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizerSCI.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerSCI.depthBiasEnable         = VK_FALSE;
    rasterizerSCI.depthBiasConstantFactor = 0.0f;
    rasterizerSCI.depthBiasClamp          = 0.0f;
    rasterizerSCI.depthBiasSlopeFactor    = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampleSCI = {0};
    multisampleSCI.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleSCI.sampleShadingEnable   = VK_FALSE;
    multisampleSCI.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampleSCI.minSampleShading      = 1.0f;
    multisampleSCI.pSampleMask           = 0;
    multisampleSCI.alphaToCoverageEnable = VK_FALSE;
    multisampleSCI.alphaToOneEnable      = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAS = {0};
    colorBlendAS.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAS.blendEnable    = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlendCI = {0};
    colorBlendCI.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendCI.logicOpEnable   = VK_FALSE;
    colorBlendCI.attachmentCount = 1;
    colorBlendCI.pAttachments    = &colorBlendAttachmentState;

    VkPipelineDepthStencilStateCreateInfo depthStencilSCI = {0};
    depthStencilSCI.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilSCI.depthTestEnable       = VK_TRUE;
    depthStencilSCI.depthWriteEnable      = VK_TRUE;
    depthStencilSCI.depthCompareOp        = VK_COMPARE_OP_LESS;
    depthStencilSCI.depthBoundsTestEnable = VK_FALSE;
    depthStencilSCI.minDepthBounds        = 0.0f;
    depthStencilSCI.maxDepthBounds        = 1.0f;
    depthStencilSCI.stencilTestEnable     = VK_FALSE;
    depthStencilSCI.front                 = = {0};
    depthStencilSCI.back                  = = {0};

    VkPipelineLayoutCreateInfo piplelineLayoutCI = {0};
    piplelineLayoutCI.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    piplelineLayoutCI.setLayoutCount = 1;
    piplelineLayoutCI.pSetLayouts    = &engine.descriptorSetLayout;

    VkResult result = vkCreatePipelineLayout(engine.logicalDevice, &piplelineLayoutCI,
                                             0, &engine.graphicsPipelineLayout);

    RIV_ASSERT_VK_SUCCESS(result, "failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo graphicsCI = {0};
    graphicsCI.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsCI.stageCount          = 2;
    graphicsCI.pStages             = shaderStages;

    graphicsCI.pVertexInputState   = &vertexInputStateCreateInfo;
    graphicsCI.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    graphicsCI.pViewportState      = &viewportStateCreateInfo;
    graphicsCI.pRasterizationState = &rasterizerStateCreateInfo;
    graphicsCI.pMultisampleState   = &multisampleStateCreateInfo;
    graphicsCI.pDepthStencilState  = &depthStencilStateCreateInfo;
    graphicsCI.pColorBlendState    = &colorBlendStateCreateInfo;
    graphicsCI.pDynamicState       = &dynamicStateCreateInfo;

    graphicsCI.layout              = engine.graphicsPipelineLayout;
    graphicsCI.renderPass          = engine.renderPass;
    graphicsCI.subpass             = 0;

    result = vkCreateGraphicsPipelines(engine.logicalDevice, VK_NULL_HANDLE, 1,
                                       &graphicsCI, 0, &engine.graphicsPipeline);

    RV_ASSERT(result == VK_SUCCESS, "failed to create graphics pipeline!");

    vkDestroyShaderModule(engine->vkData->logicalDevice, vertShaderModule, 0);
    vkDestroyShaderModule(engine->vkData->logicalDevice, fragShaderModule, 0);
}

void createFramebuffers
(
    EngineData *engine
){
    engine.swapchainFramebuffers.resize(engine.swapchainImageViews.size());

    for(size_t i = 0; i < engine.swapchainImageViews.size(); ++i)
    {
        VkImageView attachments[2] =
        {
            engine.swapchainImageViews[i],
            engine.depthImageView
        };

        VkFramebufferCreateInfo framebufferCreateInfo = {0};
        framebufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass      = engine.renderPass;
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferCreateInfo.pAttachments    = attachments.data();
        framebufferCreateInfo.width           = engine.swapchainExtent.width;
        framebufferCreateInfo.height          = engine.swapchainExtent.height;
        framebufferCreateInfo.layers          = 1;

        VkResult result = vkCreateFramebuffer(engine.logicalDevice,
                                              &framebufferCreateInfo,
                                              0,
                                              &engine.swapchainFramebuffers[i]);

        RV_ASSERT(result == VK_SUCCESS, "failed to create framebuffer!");
    }
}

void recordCommandBuffer
(
    const EngineData      *engine,
    const VkCommandBuffer *commandBuffer,
    const uint32_t        *imageIndex
){
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    RIV_ASSERT_VK_SUCCESS(result, "failed to begin recording command buffer!");

    std::array<VkClearValue, 2> clearValues = {0};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo = {0};
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass        = engine.renderPass;
    renderPassInfo.framebuffer       = engine.swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = engine.swapchainExtent;
    renderPassInfo.clearValueCount   = (u32)(clearValues.size());
    renderPassInfo.pClearValues      = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine.graphicsPipeline);

    VkBuffer vertexBuffers[] =
    {
        engine.vertexBuffer
    };
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, engine.vertexBuffer, engine.vertSize, VK_INDEX_TYPE_UINT32);

    VkViewport viewport = {0};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = (f32)(engine->vkData->swapchainExtent.width);
    viewport.height   = (f32)(engine->vkData->swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.offset = {0, 0};
    scissor.extent = engine.swapchainExtent;

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            engine->vkData->graphicsPipelineLayout, 0, 1,
                            &engine.descriptorSets[currentFrame], 0, 0);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(engine.vertexIndices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    result = vkEndCommandBuffer(commandBuffer);
    RIV_ASSERT_VK_SUCCESS(result, "failed to end recording command buffer!");
}

void createCommandPools
(
    EngineData *engine
){
    VkCommandPoolCreateInfo graphicsPoolInfo = {0};
    graphicsPoolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    graphicsPoolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    graphicsPoolInfo.queueFamilyIndex = engine.logicalQueueFamilies.graphicsIndex;

    VkResult result = vkCreateCommandPool(engine.logicalDevice, &graphicsPoolInfo, 0, &engine.graphicsCommandPool);
    RIV_ASSERT_VK_SUCCESS(result, "failed to create graphics command pool!");

    VkCommandPoolCreateInfo transferPoolInfo = {0};
    transferPoolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transferPoolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    transferPoolInfo.queueFamilyIndex = engine.logicalQueueFamilies.transferIndex;

    result = vkCreateCommandPool(engine.logicalDevice, &transferPoolInfo, 0,
                                 &engine.transferCommandPool);
    RV_ASSERT(result == VK_SUCCESS, "failed to create transfer command pool!");
}

VkCommandBuffer setupCommandBuffer
(
    const EngineData *engine,
    VkCommandPool    *commandPool
){
    VkCommandBufferAllocateInfo commandbufAllocInfo = {0};
    commandbufAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandbufAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandbufAllocInfo.commandPool        = commandPool;
    commandbufAllocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = {0};
    VkResult result = vkAllocateCommandBuffers(engine.logicalDevice,
                                               &commandbufAllocInfo, &commandBuffer);
    RV_ASSERT(result == VK_SUCCESS, "failed to allocate single time command buffer!");

    VkCommandBufferBeginInfo commandbufBeginInfo = {0};
    commandbufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandbufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &commandbufBeginInfo);

    return commandBuffer;
}

void flushCommandBuffer
(
    const EngineData      *engine,
    const VkCommandBuffer *commandBuffer,
    const VkCommandPool   *commandPool,
    const VkQueue         *queue
){
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo singleTimeSubmitInfo = {0};
    singleTimeSubmitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    singleTimeSubmitInfo.commandBufferCount = 1;
    singleTimeSubmitInfo.pCommandBuffers    = &commandBuffer;

    VkFence queueFence = 0;
    VkFenceCreateInfo fenceCreateInfo = {0};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkResult result = vkCreateFence(engine.logicalDevice, &fenceCreateInfo, 0, &queueFence);
    RV_ASSERT(result == VK_SUCCESS, "failed to create queue fence!");

    vkQueueSubmit(queue, 1, &singleTimeSubmitInfo, queueFence);

    vkWaitForFences(engine.logicalDevice, 1, &queueFence, VK_TRUE, UINT64_MAX);

    vkFreeCommandBuffers(engine.logicalDevice, commandPool, 1, &commandBuffer);
    vkDestroyFence(engine.logicalDevice, queueFence, 0);
}

void createCommandBuffers
(
    EngineData *engine
){
    engine->vkData->commandBuffers.reserve(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = engine.graphicsCommandPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (u32)(pdArrSize(engine.commandBuffers);

    VkResult result = vkAllocateCommandBuffers(engine.logicalDevice, &allocInfo,
                                               engine.commandBuffers);
    RV_ASSERT(result == VK_SUCCESS, "failed to allocate command buffers!");
}

void cleanupSyncObjects
(
    EngineData *engine
){
    for(u64 i = 0; i < pdArrSize(engine.swapchainImages); ++i)
    {
        vkDestroySemaphore(engine.logicalDevice, engine.imageReadyForWriteSemaphores[i], 0);
        engine.imageReadyForWriteSemaphores[i] = VK_NULL_HANDLE;
        RV_TRACE("destroyed imageReadyForWriteSemaphore No. %"PRIu64, i);

        vkDestroySemaphore(engine.logicalDevice, engine.imageReadyForPresentSemaphores[i], 0);
        engine.imageReadyForPresentSemaphores[i] = VK_NULL_HANDLE;
        riverLog("destroyed imageReadyForPresentSemaphore No. %"PRIu64, i);

        vkDestroyFence(engine.logicalDevice, engine.inFlightFences[i], 0);
        engine.inFlightFences[i] = VK_NULL_HANDLE;
        riverLog("destroyed inFlightFence No. %"PRIu64, i);
    }

    vkDestroySemaphore(engine->vkData->logicalDevice, engine->vkData->acquireSemaphore,
                       0);
    engine.acquireSemaphore = VK_NULL_HANDLE;
    RV_TRACE("destroyed acquireSemaphore.");
}

void createSyncObjects
(
    EngineData *engine
){
    // TODO: make these dynamic arrays and set the null handles manually
    engine->vkData->imageReadyForWriteSemaphores.resize(engine.swapchainImages.size(), VK_NULL_HANDLE);
    engine->vkData->imageReadyForPresentSemaphores.resize(engine.swapchainImages.size(), VK_NULL_HANDLE);
    engine->vkData->inFlightFences.resize(engine.swapchainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult result = vkCreateSemaphore(engine->vkData->logicalDevice,
                                        &semaphoreInfo, 0, &engine.acquireSemaphore);
    RV_ASSERT(result == VK_SUCCESS, "failed to create acquireSemaphore.");

    for(u64 i = 0; i < pdArrSize(engine->vkData->swapchainImages); ++i)
    {
        VkResult result = vkCreateSemaphore(engine->vkData->logicalDevice,
                                            &semaphoreInfo, 0,
                                            &engine->vkData->imageReadyForPresentSemaphores[i]);

        RV_ASSERT(result == VK_SUCCESS,
                  "failed to create imageReadyForPresentSemaphore No. %" PRIu64, i);

        if(engine->vkData->imageReadyForWriteSemaphores[i] == VK_NULL_HANDLE)
        {
            VkResult result = vkCreateSemaphore(engine.logicalDevice,
                                                &semaphoreInfo,
                                                0,
                                                &engine.imageReadyForWriteSemaphores[i]);

            RV_ASSERT(result, "failed to create imageReadyForWriteSemaphore No. %"
                      PRIu64, i);
        }

        if(engine.inFlightFences[i] == VK_NULL_HANDLE)
        {
            VkResult result = vkCreateFence(engine.logicalDevice, &fenceInfo, 0, &engine.inFlightFences[i]);
            RV_ASSERT(result == VK_SUCCESS, "failed to create inFlightFence No. %"PRIu64, i);
        }
    }
}

void createDescriptorSetLayout
(
    EngineData &engine
){
    VkDescriptorSetLayoutBinding uniformBufferLayoutBinding = {0};
    uniformBufferLayoutBinding.binding            = 0;
    uniformBufferLayoutBinding.descriptorCount    = 1;
    uniformBufferLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBufferLayoutBinding.pImmutableSamplers = 0;
    uniformBufferLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding textureSamplerLayoutBinding = {0};
    textureSamplerLayoutBinding.binding            = 1;
    textureSamplerLayoutBinding.descriptorCount    = 1;
    textureSamplerLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureSamplerLayoutBinding.pImmutableSamplers = 0;
    textureSamplerLayoutBinding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings =
    {
        uniformBufferLayoutBinding,
        textureSamplerLayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo dscLCI = {0};
    dscLCI.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dscLCI.bindingCount = static_cast<uint32_t>(bindings.size());
    dscLCI.pBindings    = bindings.data();

    VkResult result = vkCreateDescriptorSetLayout(engine.logicalDevice, &dscLCI, 0,
                                                  &engine.descriptorSetLayout);

    RIV_ASSERT_VK_SUCCESS(result, "failed to create descriptor set layout!");
}

void createDescriptorPool
(
    EngineData &engine
){
    VkDescriptorPoolSize poolSizes[2] = {0};
    poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (u32)(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = (u32)(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {0};
    descriptorPoolCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolCreateInfo.pPoolSizes    = poolSizes.data();
    descriptorPoolCreateInfo.maxSets       = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptorPoolCreateInfo.flags         = 0;

    VkResult result = vkCreateDescriptorPool(engine->vkData->logicalDevice,
                                             &descriptorPoolCreateInfo, 0,
                                             &engine->vkData->descriptorPool);
    RV_ASSERT(result == VK_SUCCESS, "failed to create descriptor pool.");
}

void createDescriptorSets
(
    EngineData &engine
){
    std::vector<VkDescriptorSetLayout> setLayouts(MAX_FRAMES_IN_FLIGHT, engine.descriptorSetLayout);

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {0};
    descriptorSetAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool     = engine.descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptorSetAllocInfo.pSetLayouts        = setLayouts.data();

    engine.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    VkResult result = vkAllocateDescriptorSets(engine.logicalDevice, &descriptorSetAllocInfo, engine.descriptorSets.data());
    RV_ASSERT(result == VK_SUCCESS, "failed to allocate descriptor sets!");

    for(size i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VkDescriptorBufferInfo descriptorBufferInfo = {0};
        descriptorBufferInfo.buffer = engine.uniformBuffers[i];
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range  = sizeof(UniformBufferObject);

        VkDescriptorImageInfo descriptorImageInfo = {0};
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptorImageInfo.imageView   = engine.textureImageView;
        descriptorImageInfo.sampler     = engine.textureSampler;

        VkWriteDescriptorSet uniformWriteDescriptorSet = {0};
        uniformWriteDescriptorSet.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uniformWriteDescriptorSet.dstSet           = engine.descriptorSets[i];
        uniformWriteDescriptorSet.dstBinding       = 0;
        uniformWriteDescriptorSet.dstArrayElement  = 0;
        uniformWriteDescriptorSet.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformWriteDescriptorSet.descriptorCount  = 1;
        uniformWriteDescriptorSet.pBufferInfo      = &descriptorBufferInfo;
        uniformWriteDescriptorSet.pImageInfo       = 0;
        uniformWriteDescriptorSet.pTexelBufferView = 0;

        VkWriteDescriptorSet samplerWriteDescriptorSet = {0};
        samplerWriteDescriptorSet.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        samplerWriteDescriptorSet.dstSet           = engine.descriptorSets[i];
        samplerWriteDescriptorSet.dstBinding       = 1;
        samplerWriteDescriptorSet.dstArrayElement  = 0;
        samplerWriteDescriptorSet.descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerWriteDescriptorSet.descriptorCount  = 1;
        samplerWriteDescriptorSet.pBufferInfo      = 0;
        samplerWriteDescriptorSet.pImageInfo       = &descriptorImageInfo;
        samplerWriteDescriptorSet.pTexelBufferView = 0;

        std::array<VkWriteDescriptorSet, 2> descriptorSetWrites =
        {
            uniformWriteDescriptorSet,
            samplerWriteDescriptorSet
        };

        vkUpdateDescriptorSets(engine.logicalDevice,
                               static_cast<uint32_t>(descriptorSetWrites.size()),
                               descriptorSetWrites.data(),
                               0,
                               0);
    }
}
