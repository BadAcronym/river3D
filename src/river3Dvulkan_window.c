#include "river3D_main.h"

f_internal void framebufferResizeCallback
(
    // GLFWwindow *window,
    // int        width,
    // int        height
){
    // EngineData *engine = static_cast<EngineData*>(glfwGetWindowUserPointer(window));

    // engine->framebufferResized = VK_TRUE;
}

void createSurface
(
    EngineData *engine
){
    // TODO: find out if I can create a surface smaller than the window.

    VkResult result = glfwCreateWindowSurface(engine->vkData->instance,
                                              engine->window, 0,
                                              &engine->vkData->surface);
    RV_ASSERT(result == VK_SUCCESS, "failed to create window surface!");
}
