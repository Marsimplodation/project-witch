#include "SoulShard.h"

#include "Editor/Editor.h"
#include "GLFW/glfw3.h"
#include "InputHandling/InputHandling.h"
#include "Vulkan/VkRenderer.h"
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <vulkan/vulkan_core.h>


int SoulShard::startup() {
    renderer.data = {
        .vertShaderPath = "./vert.spv",
        .fragShaderPath = "./frag.spv",
        .shadowVertShaderPath = "./shadowVert.spv",
        .shadowFragShaderPath = "./shadowFrag.spv",
    };

    if (0 != renderer.deviceInitialization()) return -1;
    if (0 != renderer.createSwapchain()) return -1;
    if (0 != renderer.getQueues()) return -1;
    if (0 != renderer.createRenderPass()) return -1;
    if (0 != renderer.createDescriptorPool()) return -1;
    if (0 != renderer.createDescriptorLayout()) return -1;
    
    if (0 != renderer.createRenderingPipeline()) return -1;
    if (0 != renderer.createFramebuffers()) return -1;
    if (0 != renderer.createCommandPool()) return -1;

    
    if (0 != renderer.createCommandBuffers()) return -1;
    if (0 != renderer.createSyncObjects()) return -1;
    inputHandler.init(renderer.init.window);
    renderer.enginePtr = this;
    scene.enginePtr = this;
    renderer.data.editorMode = true;
    renderer.loadTexture("./textures/dummy.png");

    return 0;
};


int SoulShard::run() {
    //testPhysics();
    renderer.data.vertices = &gpuGeometry.vertices;
    renderer.data.indices = &gpuGeometry.indices;
    //std::thread thread(runAt60fps);

    if (0 != renderer.createGeometryBuffers()) return -1;
    if (0 != renderer.createUniformBuffers()) return -1;
    auto lastTime = glfwGetTime(); 
    renderer.data.gui = ImguiModule();
    renderer.data.gui.init(&renderer.init, &renderer.data);
    renderer.data.gui.enginePtr = this;

    while (!glfwWindowShouldClose(renderer.init.window)) {
        auto currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        glfwPollEvents();
        scene.updateModels();
        if (inputHandler.isKeyPressedOnce(KEY_U)) {
            renderer.data.editorMode = !renderer.data.editorMode;
            inputHandler.releaseMouse();
        }

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

        int res = renderer.drawFrame();
        if (res != 0) {
            std::cout << "failed to draw frame \n";
            return -1;
        }
        inputHandler.update();
    }
    renderer.init.disp.deviceWaitIdle();

    renderer.data.gui.destroy(renderer.init.device);
    renderer.cleanup();
    //thread.join();
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
