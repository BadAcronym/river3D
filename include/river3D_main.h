#pragma once

#include "pd_string_view.h"

#include <vulkan/vulkan_core.h>

#define bool  _Bool
#define true  1
#define false 0

#define v_persistent  static
#define s_global      static
#define f_internal    static

#define ENGINE_NAME "river3D";
//maybe get rid of this in the future
#define MAX_FRAMES_IN_FLIGHT 2;

#ifdef BUILD_LINUX
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/XKBlib.h>
    #include <X11/Xcursor/Xcursor.h>
    #include <X11/extensions/Xrender.h>

    #ifndef  __USE_POSIX199309
        #define __USE_POSIX199309
    #endif
    #include <time.h>

    #include <pthread.h>
    #define  RV_SCANLINE 32
    #define  RV_CONFIG_PATH "./.river3Dconf"

    #define RV_MOUSE1 Button1
    #define RV_MOUSE2 Button3
    #define RV_MOUSE3 Button2
    #define RV_MOUSE4 Button4
    #define RV_MOUSE5 Button5
#endif

#ifdef BUILD_WINDOWS
    #include <Windows.h>
    #define  RV_CONFIG_PATH "./river3D.ini"

    #define RV_MOUSE1 0x01
    #define RV_MOUSE2 0x02
    #define RV_MOUSE3 0x10
#endif

typedef struct Vertex
{
    i32 x;
    i32 y;
    i32 z;
    i32 textureY;
    i32 textureX;
    u32 colour;
}
Vertex;

typedef struct UniformBufferObject
{
    float model[4][4];
    float view[4][4];
    float projection[4][4];
}
UniformBufferObject;

typedef struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR       *formats;
    VkPresentModeKHR         *presentModes;
}
SwapchainSupportDetails;

typedef struct QueueFamilyIndices
{
    u32 graphicsIndex; // = UINT32_MAX;
    u32 transferIndex; // = UINT32_MAX;
    u32 presentIndex;  // = UINT32_MAX;
}
QueueFamilyIndices;

typedef struct RiverConfig
{
    u08        renderer;
    u08        backgrounds;
    u32        windowHeight;
    u32        windowWidth;
    u32        canvasHidth;
    u32        canvasHeight;
    u32        choices;
    StringView projectName; // = "RV_UNINITIALIZED_STRING";
    StringView projectVersion;     // = "RV_UNINITIALIZED_STRING";
    StringView projectLog;         // = "RV_UNINITIALIZED_PATH";
    StringView projectModelPath;   // = "RV_UNINITIALIZED_PATH";
    StringView projectTexturePath; // = "RV_UNINITIALIZED_PATH";
    StringView vertexShader;       // = "RV_UNINITIALIZED_PATH";
    StringView fragmentShader;     // = "RV_UNINITIALIZED_PATH";
}
RiverConfig;

typedef struct VulkanData
{
    #ifdef DEBUG
    VkDebugUtilsMessengerEXT         debugMessenger;
    #endif

    VkPresentModeKHR                 presentMode;
    VkInstance                       instance;
    VkSurfaceKHR                     surface;
    QueueFamilyIndices               logicalQueueFamilies;
    VkPhysicalDevice                 physicalDevice;
    VkPhysicalDeviceProperties       deviceProperties;
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    VkPhysicalDeviceFeatures         deviceFeatures;
    VkDevice                         logicalDevice;
    VkSwapchainKHR                   swapchain;
    VkFormat                         swapchainImageFormat;
    VkExtent2D                       swapchainExtent;
    u32                              swapchainImageCount;
    VkImage                          *swapchainImages;
    VkImageView                      *swapchainImageViews;
    VkFramebuffer                    *swapchainFramebuffers;
    VkRenderPass                     renderPass;
    VkPipeline                       graphicsPipeline;
    VkPipelineLayout                 graphicsPipelineLayout;
    VkDescriptorSetLayout            descriptorSetLayout;
    VkDescriptorSet                  *descriptorSets;
    VkCommandPool                    graphicsCommandPool;
    VkCommandPool                    transferCommandPool;
    VkDescriptorPool                 descriptorPool;
    VkCommandBuffer                  commandBuffers;
    VkQueue                          graphicsQueue;
    VkQueue                          presentQueue;
    VkQueue                          transferQueue;
    VkSemaphore                      *imageReadyForWriteSemaphores;
    VkSemaphore                      *imageReadyForPresentSemaphores;
    VkSemaphore                      acquireSemaphore;
    VkFence                          *inFlightFences;
    Vertex                           *vertices;
    u32                              *vertexIndices;
    VkBuffer                         vertexBuffer;
    VkDeviceMemory                   vertexBufferMemory;
    VkDeviceSize                     vertSize;
    VkImage                          depthImage;
    VkDeviceMemory                   depthImageMemory;
    VkImageView                      depthImageView;
    VkBuffer                         *uniformBuffers;
    VkDeviceMemory                   *uniformBuffersMemory;
    void                             *uniformBuffersMapped;
    u32                              mipLevels;
    VkImage                          textureImage;
    VkImageView                      textureImageView;
    VkDeviceMemory                   textureImageMemory;
    VkSampler                        textureSampler;
    VkSampleCountFlagBits            msaaSamples;
}
VulkanData;

typedef struct RiverImage
{
    StringView path;
    u08        *data;
    u08        channels;
    u32        width;
    u32        height;

    #ifdef BUILD_LINUX
    Pixmap     pixmap;
    Picture    picture;
    #endif

    #ifdef BUILD_WINDOWS
    BITMAPINFO info;
    #endif
}
RiverImage;

typedef struct EngineData
{
    StringView        windowName;
    bool              framebufferResized;
    VulkanData        *vkData;
    RiverConfig       config;

#ifdef BUILD_LINUX
    Display           *display;
    Screen            *screen;
    XRenderPictFormat *format;
    Visual            *visual;
    Window            window;
    GC                context;
    Picture           blitDstPict;
#endif

#ifdef BUILD_WINDOWS
    HINSTANCE         instance;
    HWND              window;
    HDC               context;
    HBITMAP           cursorBitmap;
    HBITMAP           cursorMask;
    HCURSOR           hCursor;
#endif
}
EngineData;

extern void vkInit
(
    EngineData *engine
);

extern void vkShutdown
(
    EngineData *engine
);

extern void drawFrame
(
    EngineData *engine
);

extern void riverSetupLog
(
    const StringView path
);

extern void riverCloseLog
(
    void
);

#ifdef DEBUG

#define VALIDATION_LAYER_COUNT 4

const char* validationLayers[VALIDATION_LAYER_COUNT] =
{
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_KHRONOS_synchronization2",
    // "VK_LAYER_LUNARG_crash_diagnostic",
    "VK_LAYER_LUNARG_monitor",
    "VK_LAYER_RTSS"
};
#endif

extern void riverLog
(
    const StringView text,
    const u08        level
);
