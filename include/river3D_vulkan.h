#ifndef RV3D_VULKAN_H
#define RV3D_VULKAN_H

#include "river3D_main.h"

typedef PFN_vkCreateDebugUtilsMessengerEXT  vkCreateMsgr;
typedef PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyMsgr;

extern void createSurface
(
    EngineData *engine
);

extern SwapchainSupportDetails querySwapchainSupport
(
    const VkPhysicalDevice *physicalDevice,
    const VkSurfaceKHR     *surface
);

extern QueueFamilyIndices findQueueFamilies
(
    const EngineData *engine
);

extern void pickPhysicalDevice
(
    EngineData *engine
);

extern void createLogicalDevice
(
    EngineData *engine
);

extern VkSampleCountFlagBits getMaxMSAASamples
(
    const EngineData *engine
);

extern void loadModel
(
    EngineData *engine,
    StringView modelPath
);

extern void createBuffer
(
    const EngineData            *engine,
    const VkDeviceSize          *bufferSize,
    const VkBufferUsageFlags    *usageFlags,
    const VkMemoryPropertyFlags *memPropFlags,
    VkBuffer                    *buffer,
    VkDeviceMemory              *bufferMemory,
    const u32                   *uniqueQueueFamilies // was a const std::set
);

extern void createDepthResources
(
    EngineData *engine
);

extern u32 findSuitableMemoryType
(
    const EngineData            *engine,
    const u32                   *typeFilter,
    const VkMemoryPropertyFlags *flags
);

extern void createVertexBuffer
(
    EngineData *engine
);

extern void createUniformBuffers
(
    EngineData *engine
);

extern VkFormat findSupportedFormat
(
    const EngineData           *engine,
    const VkFormat             *candidates,
    const VkImageTiling        *tiling,
    const VkFormatFeatureFlags *features
);

extern void updateUniformBuffer
(
    const EngineData *engine,
    u32              currentImage //redundant?
);

extern VkVertexInputBindingDescription getVertexBindingDescription
(
    void
);

extern VkVertexInputAttributeDescription* getVertexAttributeDescriptions
(
    void
);

extern void createImage
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
);

extern VkImageView createImageView
(
    const EngineData         *engine,
    const VkImage            *image,
    const u32                *mipLevels,
    const VkFormat           *format,
    const VkImageAspectFlags *aspectFlags
);

extern void createTextureImage
(
    EngineData *engine
);

extern void createTextureSampler
(
    EngineData *engine
);

extern void transitionImageLayout
(
    EngineData          *engine,
    const VkImage       *image,
    const u32           *mipLevels,
    const VkFormat      *format,
    const VkImageLayout *oldLayout,
    const VkImageLayout *newLayout
);

extern void createGraphicsPipeline
(
    EngineData *engine
);

extern void createFramebuffers
(
    EngineData *engine
);

extern void createCommandPools
(
    EngineData *engine
);

extern void createCommandBuffers
(
    EngineData *engine
);

extern VkCommandBuffer setupCommandBuffer
(
    const EngineData *engine,
    VkCommandPool    *commandPool
);

extern void flushCommandBuffer
(
    const EngineData      *engine,
    const VkCommandBuffer *commandBuffer,
    const VkCommandPool   *commandPool,
    const VkQueue         *queue
);

extern void createDescriptorSetLayout
(
    EngineData *engine
);

extern void createDescriptorPool
(
    EngineData *engine
);

extern void createDescriptorSets
(
    EngineData *engine
);

extern void createSyncObjects
(
    EngineData *engine
);

extern void cleanupSyncObjects
(
    EngineData *engine
);

extern void recordCommandBuffer
(
    const EngineData      *engine,
    const VkCommandBuffer *commandBuffer,
    const u32             *imageIndex
);

extern void createSwapchain
(
    EngineData *engine
);

extern void recreateSwapchain
(
    EngineData *engine
);

extern void createRenderPass
(
    EngineData *engine
);

extern void cleanupSwapchain
(
    const EngineData *engine
);

#endif
