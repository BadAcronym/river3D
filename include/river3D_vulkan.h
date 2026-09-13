#pramga once

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
    const std::set<uint32_t>    *uniqueQueueFamilies
);

extern void createDepthResources
(
    EngineData *engine
);

extern uint32_t findSuitableMemoryType
(
    const EngineData            *engine,
    const uint32_t              *typeFilter,
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
    const EngineData            *engine,
    const std::vector<VkFormat> *candidates,
    const VkImageTiling         *tiling,
    const VkFormatFeatureFlags  *features
);

extern void updateUniformBuffer
(
    const EngineData *engine,
    uint32_t         currentImage //redundant?
);

extern VkVertexInputBindingDescription getVertexBindingDescription();
extern VkVertexInputAttributeDescription* getVertexAttributeDescriptions();

extern void createImage
(
    const EngineData            *engine,
    const uint32_t              *width,
    const uint32_t              *height,
    const uint32_t              *mipLevels,
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
    const uint32_t           *mipLevels,
    const VkFormat           *format,
    const VkImageAspectFlags *aspectFlags
);

extern void createTextureImage
(
    EngineData            *engine,
    const ProjectManifest *manifest
);

extern void createTextureSampler
(
    EngineData *engine
);

extern void transitionImageLayout
(
    EngineData          *engine,
    const VkImage       *image,
    const uint32_t      *mipLevels,
    const VkFormat      *format,
    const VkImageLayout *oldLayout,
    const VkImageLayout *newLayout
);

extern void createGraphicsPipeline
(
    EngineData            *engine,
    const ProjectManifest *manifest
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
    const EngineData    *engine,
    VkCommandPool       &commandPool
);

extern void flushCommandBuffer
(
    const EngineData        *engine,
    const VkCommandBuffer   &commandBuffer,
    const VkCommandPool     &commandPool,
    const VkQueue           &queue
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
    const uint32_t        *imageIndex
);

extern void createSwapchain
(
    EngineData          *engine,
    const UserSettings  *settings
);

extern void recreateSwapchain
(
    EngineData         *engine,
    const UserSettings *settings
);

extern void createRenderPass
(
    EngineData *engine
);

extern void cleanupSwapchain
(
    const EngineData *engine
);
