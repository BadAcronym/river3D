#include "river3D_main.h"

#include "vertstream_main.h"

#include <vulkan/vulkan_core.h>

void loadModel
(
    EngineData &engine,
    StringView modelPath
){
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    bool result = tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &error, modelPath.string().c_str());
    RIV_ASSERT(result, (warn + error));

    std::unordered_map<Vertex, uint32_t> uniqueVertices = {0};

    for(const tinyobj::shape_t &shape : shapes)
    {
        for(const tinyobj::index_t &index : shape.mesh.indices)
        {
            Vertex vertex = {0};

            vertex.position =
            {
                attributes.vertices[3 * index.vertex_index + 0],
                attributes.vertices[3 * index.vertex_index + 1],
                attributes.vertices[3 * index.vertex_index + 2] - 0.25
            };

            vertex.textureCoordinate =
            {
                attributes.texcoords[2 * index.texcoord_index + 0],
                1.0f - attributes.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.colour = { 1.0f, 1.0f, 1.0f };

            if(!uniqueVertices.contains(vertex))
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(engine.vertices.size());
                engine.vertices.emplace_back(vertex);
            }

            engine.vertexIndices.emplace_back(uniqueVertices[vertex]);
        }
    }
}

VkVertexInputBindingDescription getVertexBindingDescription
(
    void
){
    VkVertexInputBindingDescription bindingDescription = {0};
    bindingDescription.binding   = 0;
    bindingDescription.stride    = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

VkVertexInputAttributeDescription* getVertexAttributeDescriptions
(
    void
){
    VkVertexInputAttributeDescription attributeDescriptions[3];

    attributeDescriptions[0].binding  = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset   = offsetof(Vertex, position);

    attributeDescriptions[1].binding  = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset   = offsetof(Vertex, colour);

    attributeDescriptions[2].binding  = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset   = offsetof(Vertex, textureCoordinate);

    return attributeDescriptions;
}

uint32_t findSuitableMemoryType
(
    const EngineData            *engine,
    const uint32_t              *typeFilter,
    const VkMemoryPropertyFlags *flags
){
    for(uint32_t i = 0; i < engine.deviceMemoryProperties.memoryTypeCount; ++i)
    {
        if(typeFilter & (1 << i) &&
          ((engine.deviceMemoryProperties.memoryTypes[i].propertyFlags & flags) == flags))
        {
            return i;
        }
    }

    riverLog("failed to find suitable memory type!", RIV_LOG_LEVEL_ERROR);
    return UINT32_MAX;
}

void createBuffer
(
    const EngineData            *engine,
    const VkDeviceSize          *bufferSize,
    const VkBufferUsageFlags    *usageFlags,
    const VkMemoryPropertyFlags *memPropFlags,
    VkBuffer                    *buffer,
    VkDeviceMemory              *bufferMemory,
    const std::set<uint32_t>    *uniqueQueueFamilies
){
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size  = bufferSize;
    bufferInfo.usage = usageFlags;

    if(uniqueQueueFamilies.size() > 1)
    {
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    }
    else
    {
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    std::vector<uint32_t> queueFamilyIndices(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());

    bufferInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
    bufferInfo.pQueueFamilyIndices = queueFamilyIndices.data();

    VkResult result = vkCreateBuffer(engine.logicalDevice, &bufferInfo, nullptr, &buffer);
    RIV_ASSERT_VK_SUCCESS(result, "failed to create buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(engine.logicalDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findSuitableMemoryType(engine, memRequirements.memoryTypeBits, memPropFlags);

    result = vkAllocateMemory(engine.logicalDevice, &allocInfo, nullptr, &bufferMemory);
    RIV_ASSERT_VK_SUCCESS(result, "failed to allocate buffer!");

    vkBindBufferMemory(engine.logicalDevice, buffer, bufferMemory, 0);
}

internal void copyBuffer
(
    EngineData         *engine,
    const VkBuffer     *srcBuffer,
    const VkBuffer     *dstBuffer,
    const VkDeviceSize *bufferSize
){
  VkCommandBuffer commandBuffer = setupCommandBuffer(engine,
                                                     engine.transferCommandPool);

  VkBufferCopy transferCopyRegion = {0};
  transferCopyRegion.size = bufferSize;
  transferCopyRegion.srcOffset = 0;
  transferCopyRegion.dstOffset = 0;

  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &transferCopyRegion);

  flushCommandBuffer(engine, commandBuffer, engine.transferCommandPool,
                     engine.transferQueue);
}

void createVertexBuffer
(
    EngineData *engine
){
    engine.vertSize = sizeof(engine.vertices[0]) * engine.vertices.size();
    VkDeviceSize indexSize = sizeof(engine.vertexIndices[0]) *
                                    engine.vertexIndices.size();

    VkDeviceSize bufferSize = engine.vertSize + indexSize;

    VkBuffer       stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    uint32_t queueFamilies[2] =
    {
        engine.logicalQueueFamilies.graphicsIndex,
        engine.logicalQueueFamilies.transferIndex
    };

    createBuffer(engine,
                 bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory,
                 queueFamilies);

    void* pData;

    vkMapMemory(engine.logicalDevice, stagingBufferMemory, 0, engine.vertSize, 0, &pData);
    memcpy(pData, engine.vertices.data(), static_cast<size_t>(engine.vertSize));
    vkUnmapMemory(engine.logicalDevice, stagingBufferMemory);

    vkMapMemory(engine.logicalDevice, stagingBufferMemory, engine.vertSize, indexSize, 0, &pData);
    memcpy(pData, engine.vertexIndices.data(), static_cast<size_t>(indexSize));
    vkUnmapMemory(engine.logicalDevice, stagingBufferMemory);

    createBuffer(engine,
                 bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 engine.vertexBuffer,
                 engine.vertexBufferMemory,
                 queueFamilies);

    copyBuffer(engine, stagingBuffer, engine.vertexBuffer, bufferSize);

    vkDestroyBuffer(engine.logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(engine.logicalDevice, stagingBufferMemory, nullptr);
}

void createUniformBuffers
(
    EngineData *engine
){
    VkDeviceSize uniformBufferSize = sizeof(UniformBufferObject);

    engine.uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    engine.uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    engine.uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        std::set<uint32_t> uniqueFamilyIndices =
        {
            engine.logicalQueueFamilies.graphicsIndex,
            engine.logicalQueueFamilies.presentIndex
        };

        createBuffer(engine,
                     uniformBufferSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     engine.uniformBuffers[i],
                     engine.uniformBuffersMemory[i],
                     uniqueFamilyIndices);

        vkMapMemory(engine.logicalDevice,
                    engine.uniformBuffersMemory[i],
                    0,
                    uniformBufferSize,
                    0,
                    &engine.uniformBuffersMapped[i]);
    };
}

void updateUniformBuffer
(
    const EngineData &engine,
    uint32_t         currentImage //redundant?
){
    persistent std::chrono::time_point startTime = std::chrono::high_resolution_clock::now();

    std::chrono::time_point currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject uniformBuffer = {0};
    uniformBuffer.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    uniformBuffer.view = glm::lookAt(glm::vec3(1.0f, 0.0f, 0.3f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    uniformBuffer.projection =  glm::perspective(glm::radians(35.0f),
                                                 engine.swapchainExtent.width /
                                                    static_cast<float>(engine.swapchainExtent.height),
                                                 0.1f,
                                                 256.0f);

    uniformBuffer.projection[1][1] *= -1;

    ::memcpy(engine.uniformBuffersMapped[currentImage], &uniformBuffer, sizeof(uniformBuffer));
}

VkFormat findSupportedFormat
(
    const EngineData            &engine,
    const std::vector<VkFormat> &candidates,
    const VkImageTiling         &tiling,
    const VkFormatFeatureFlags  &features
){
    for(VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(engine.physicalDevice, format, &props);

        if(tiling == VK_IMAGE_TILING_LINEAR &&
          ((props.linearTilingFeatures & features) == features))
        {
            return format;
        }
        else if(tiling == VK_IMAGE_TILING_OPTIMAL &&
               ((props.optimalTilingFeatures & features) == features))
        {
            return format;
        }
    }
    riverThrow("failed to find supported format!");
    return VK_FORMAT_MAX_ENUM; //to silence clang
}

void createDepthResources
(
    EngineData &engine
){
    const std::vector<VkFormat> candidates =
    {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    const VkFormat depthFormat = findSupportedFormat(engine,
                                                     candidates,
                                                     VK_IMAGE_TILING_OPTIMAL,
                                                     VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    createImage(engine,
                engine.swapchainExtent.width,
                engine.swapchainExtent.height,
                1,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                engine.depthImage,
                engine.depthImageMemory);

    engine.depthImageView = createImageView(engine,
                                            engine.depthImage,
                                            1,
                                            depthFormat,
                                            VK_IMAGE_ASPECT_DEPTH_BIT);

    transitionImageLayout(engine,
                          engine.depthImage,
                          1,
                          depthFormat,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}
