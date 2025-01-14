#include "SoulShard.h"

#include "Editor/Editor.h"
#include "GLFW/glfw3.h"
#include "InputHandling/InputHandling.h"
#include "Physics/JoltImpl.h"
#include "Vulkan/VkRenderer.h"
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <vulkan/vulkan_core.h>


int SoulShard::startup() {
    renderer.data = {
        .vertShaderPath = "./vert.spv",
        .fragShaderPath = "./frag.spv",
    };

    if (0 != renderer.device_initialization()) return -1;
    if (0 != renderer.create_swapchain()) return -1;
    if (0 != renderer.get_queues()) return -1;
    if (0 != renderer.createRenderPass()) return -1;
    if (0 != renderer.create_descriptor_pool()) return -1;
    if (0 != renderer.create_descriptor_layout()) return -1;
    
    if (0 != renderer.create_graphics_pipeline()) return -1;
    if (0 != renderer.create_framebuffers()) return -1;
    if (0 != renderer.create_command_pool()) return -1;

    
    if (0 != renderer.create_command_buffers()) return -1;
    if (0 != renderer.create_sync_objects()) return -1;
    inputHandler.init(renderer.init.window);
    renderer.enginePtr = this;
    renderer.data.editorMode = true;
    return 0;
};


int SoulShard::run() {
    testPhysics();
    renderer.data.vertices = &gpuGeometry.vertices;
    renderer.data.indices = &gpuGeometry.indices;

    if (0 != renderer.createGeometryBuffers()) return -1;
    if (0 != renderer.createUniformBuffers()) return -1;
    auto lastTime = glfwGetTime(); 
    renderer.data.gui = ImguiModule();
    renderer.data.gui.init(&renderer.init, &renderer.data);
    renderer.data.gui.enginePtr = this;

    while (!glfwWindowShouldClose(renderer.init.window)) {
        glfwPollEvents();
        scene.updateModels();
        if (inputHandler.isKeyPressedOnce(KEY_U)) {
            renderer.data.editorMode = !renderer.data.editorMode;
            inputHandler.releaseMouse();
        }

        auto currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;
        if (!renderer.data.editorMode) {
            renderingResolution[0] = renderer.init.swapchain.extent.width;
            renderingResolution[1] = renderer.init.swapchain.extent.height;
        } else {
            renderingResolution[0] = renderer.data.gui.previewSize.x; 
            renderingResolution[1] = renderer.data.gui.previewSize.y;
        }

        if(!renderer.data.editorMode) {
            for(auto & system : systems) {
                if(system.active)system.func(deltaTime);
            }
            renderer.updateCameraBuffer(mainCamera);
        } else {
            for(auto & system : systems) {
                if(system.active)system.func(deltaTime);
            }
            renderer.updateCameraBuffer(editorCamera);
        }

        int res = renderer.draw_frame();
        if (res != 0) {
            std::cout << "failed to draw frame \n";
            return -1;
        }
        inputHandler.update();
    }
    renderer.init.disp.deviceWaitIdle();

    renderer.data.gui.destroy(renderer.init.device);
    renderer.cleanup();
    return 0;
}

void SoulShard::destroy() {
    renderer.init.disp.deviceWaitIdle();

    renderer.data.gui.destroy(renderer.init.device);
    renderer.cleanup();
    exit(0);
}

void SoulShard::registerSystem(void(*func)(float deltaTime), const char * name) {
    systems.push_back({.active = true,
        .func = func,
        .name = std::string(name)
    });
}
