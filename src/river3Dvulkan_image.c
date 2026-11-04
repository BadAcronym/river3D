#include "river3D_main.h"
#include "rv_print_macros.h"

#include "imgsurf_main.h"

#include "vulkan/vulkan_core.h"

f_internal void copyBufferToImage
(
    EngineData     *engine,
    const VkBuffer *buffer,
    const VkImage  *image,
    const u32      *width,
    const u32      *height
){
    VkCommandBuffer commandBuffer = setupCommandBuffer(engine, engine.graphicsCommandPool);

    VkBufferImageCopy region = {0};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageOffset                 = { 0, 0, 0 };
    region.imageExtent                 = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, *buffer, *image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    flushCommandBuffer(engine, commandBuffer, engine->graphicsCommandPool,
                       engine->graphicsQueue);
}

void transitionImageLayout
(
    EngineData          *engine,
    const VkImage       *image,
    const u32           *mipLevels,
    const VkFormat      *format,
    const VkImageLayout *oldLayout,
    const VkImageLayout *newLayout
){
    VkCommandBuffer commandBuffer = setupCommandBuffer(engine, engine.graphicsCommandPool);

    VkImageMemoryBarrier barrier = {0};
    barrier.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout            = *oldLayout;
    barrier.newLayout            = *newLayout;
    barrier.srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                = *image;

    if(*newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if(*format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           *format == VK_FORMAT_D24_UNORM_S8_UINT
        ){
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if(*oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
       *newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    ){
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(*oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            *newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    ){
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if(*oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            *newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        RV_ERROR("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, 0, 0, 0, 1,
                         &barrier);

    flushCommandBuffer(engine, commandBuffer, engine->vkData->graphicsCommandPool,
                       engine->vkData->graphicsQueue);
}

f_internal void generateMipmaps
(
    EngineData    *engine,
    const VkImage *image,
    const u32     *mipLevels,
    const i32     *texWidth,
    const i32     *texHeight
){
    VkCommandBuffer commandBuffer =
        setupCommandBuffer(engine, engine->vkData->graphicsCommandPool);

    VkImageMemoryBarrier imgMB = {0};
    imgMB.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imgMB.image                           = image;
    imgMB.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    imgMB.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    imgMB.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    imgMB.subresourceRange.baseArrayLayer = 0;
    imgMB.subresourceRange.layerCount     = 1;
    imgMB.subresourceRange.levelCount     = 1;

    i32 mipWidth  = texWidth;
    i32 mipHeight = texHeight;

    for(u32 i = 1; i < mipLevels; ++i)
    {
        imgMB.subresourceRange.baseMipLevel = i - 1;
        imgMB.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgMB.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imgMB.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
        imgMB.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, 0, 0, 0, 1, &imgMB);

        VkImageBlit imageBlit = {0};
        imageBlit.srcOffsets[0]                 = {0, 0, 0};
        imageBlit.srcOffsets[1]                 =
        {
            mipWidth,
            mipHeight,
            1
        };
        imageBlit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.mipLevel       = i - 1;
        imageBlit.srcSubresource.baseArrayLayer = 0;
        imageBlit.srcSubresource.layerCount     = 1;
        imageBlit.dstOffsets[0]                 = {0, 0, 0};
        imageBlit.dstOffsets[1]                 =
        {
            mipWidth  > 1 ? mipWidth  / 2 : 1,
            mipHeight > 1 ? mipHeight / 2 : 1,
            1
        };
        imageBlit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.mipLevel       = i;
        imageBlit.dstSubresource.baseArrayLayer = 0;
        imageBlit.dstSubresource.layerCount     = 1;

        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &imageBlit,
                       VK_FILTER_LINEAR);

        imgMB.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imgMB.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imgMB.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imgMB.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0, 0, 0, 0, 0,
                             1, &imgMB);

        if(mipWidth > 1)
        {
            mipWidth /= 2;
        }

        if(mipHeight > 1)
        {
            mipHeight /= 2;
        }
    }

    imgMB.subresourceRange.baseMipLevel = mipLevels - 1;
    imgMB.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imgMB.newLayout                     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imgMB.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
    imgMB.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, 0, 0, 0,
                         1, &imageMemoryBarrier);

    flushCommandBuffer(engine, commandBuffer, engine->vkData->graphicsCommandPool,
                       engine->vkData->graphicsQueue);
}

void createTextureImage
(
    EngineData *engine,
){
    u32 texWidth;
    u32 texHeight;
    // u32 texChannels;

    u08 *pixels = imLoadFile(path, &texWidth, &texHeight, &texChannels, channels,
                             bitdepth);
    if(!pixels)
    {
        RV_ERROR("failed to load texture image '"PRI_SV"'.", PRI_SV(path));
    }

    engine.mipLevels = (u32)(floor(log2(max(texWidth, texHeight)))) + 1;

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    std::set<u32> uniqueFamilyIndices =
    {
        engine.logicalQueueFamilies.graphicsIndex,
        engine.logicalQueueFamilies.transferIndex
    };

    VkBuffer stagingBuffer = {0};
    VkDeviceMemory stagingBufferMemory = {0};

    createBuffer(engine, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory, uniqueFamilyIndices);

    void *data = 0;
    vkMapMemory(engine.logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (size)(imageSize));
    vkUnmapMemory(engine.logicalDevice, stagingBufferMemory);

    free(pixels);

    createImage(engine, texWidth, texHeight, engine.mipLevels, VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                engine.textureImage, engine.textureImageMemory);

    transitionImageLayout(engine, engine.textureImage, engine.mipLevels,
                          VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(engine, stagingBuffer, engine.textureImage, (u32)(texWidth),
                      (u32)(texHeight));

    generateMipmaps(engine, engine.textureImage, engine.mipLevels, (i32)(texWidth),
                    (i32)(texHeight));

    vkDestroyBuffer(engine.logicalDevice, stagingBuffer, 0);
    vkFreeMemory(engine.logicalDevice, stagingBufferMemory, 0);

    engine.textureImageView = createImageView(engine, engine.textureImage,
                                              engine.mipLevels, VK_FORMAT_R8G8B8A8_SRGB,
                                              VK_IMAGE_ASPECT_COLOR_BIT);
}

void createImage
(
    const EngineData            *engine,
    const u32                   *width,
    const u32                   *height,
    const u32                   *mipLevels,
    const VkFormat              *format,
    const VkImageTiling         *tiling,
    const VkImageUsageFlags     *usage,
    const VkMemoryPropertyFlags *memPropFlags,
    VkImage                     *image,
    VkDeviceMemory              *imageMem
){
    VkImageCreateInfo imageCreateInfo = {0};
    imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width  = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth  = 1;
    imageCreateInfo.mipLevels     = mipLevels;
    imageCreateInfo.arrayLayers   = 1;

    // in an edge case, this format might not be supported.
    // conversions will be done eventually
    imageCreateInfo.format        = format;
    imageCreateInfo.tiling        = tiling;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage         = usage;
    imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.flags         = 0;

    VkResult result = vkCreateImage(engine.logicalDevice, &imageCreateInfo, 0, &image);
    RIV_ASSERT_VK_SUCCESS(result, "failed to create image!");

    VkMemoryRequirements imageMemRequirements;
    vkGetImageMemoryRequirements(engine.logicalDevice, image, &imageMemRequirements);

    VkMemoryAllocateInfo mAllocInfo = {0};
    mAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mAllocInfo.allocationSize = imageMemRequirements.size;
    mAllocInfo.memoryTypeIndex = findSuitableMemoryType(engine, imageMemRequirements.memoryTypeBits, memPropFlags);

    result = vkAllocateMemory(engine.logicalDevice, &mAllocInfo, 0, &imageMem);
    RIV_ASSERT_VK_SUCCESS(result, "failed to allocate image memory!");

    vkBindImageMemory(engine.logicalDevice, image, imageMem, 0);
}

VkImageView createImageView
(
    const EngineData         *engine,
    const VkImage            *image,
    const u32                *mipLevels,
    const VkFormat           *format,
    const VkImageAspectFlags *aspectFlags
){
    VkImageViewCreateInfo viewCreateInfo = {0};
    viewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image                           = image;
    viewCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format                          = format;
    viewCreateInfo.subresourceRange.aspectMask     = aspectFlags;
    viewCreateInfo.subresourceRange.baseMipLevel   = 0;
    viewCreateInfo.subresourceRange.levelCount     = mipLevels;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount     = 1;

    VkImageView imageView;

    VkResult result = vkCreateImageView(engine.logicalDevice, &viewCreateInfo, 0, &imageView);
    RV_ASSERT(result == VK_SUCCESS, "failed to create texture image view!");

    return imageView;
}

void createTextureSampler
(
    EngineData *engine
){
    VkSamplerCreateInfo samplerCreateInfo = {0};
    samplerCreateInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter               = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter               = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.anisotropyEnable        = VK_TRUE;
    samplerCreateInfo.maxAnisotropy           = engine.deviceProperties.limits.maxSamplerAnisotropy;
    samplerCreateInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareEnable           = VK_FALSE;
    samplerCreateInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.mipLodBias              = 0.0f;
    samplerCreateInfo.minLod                  = 0.0f;
    samplerCreateInfo.maxLod                  = engine.mipLevels;

    VkResult result = vkCreateSampler(engine.logicalDevice, &samplerCreateInfo, 0, &engine.textureSampler);
    RV_ASSERT(result, "failed to create texture sampler!");
}
