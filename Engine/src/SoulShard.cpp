#include "SoulShard.h"

#include "Editor/Editor.h"
#include "GLFW/glfw3.h"
#include "InputHandling/InputHandling.h"
#include "Vulkan/VkRenderer.h"
#include "types/types.h"
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>
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

    scene.registry.registerType<TransformComponent>();

    return 0;
};

std::mutex mtx;
std::condition_variable cv;
bool frameReady = false;

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
    scene.updateModels();

    auto cpuThread = [&](){
        while (!glfwWindowShouldClose(renderer.init.window)) {
        //Timer timer("cpu thread");
        auto currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        glfwPollEvents();
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
        
        for(auto & system : systems) {
            if(system.active)system.func(deltaTime);
        }
        scene.updateModels();

        //--- UPDATE GPU STUFF ---//
        {
            std::unique_lock<std::mutex> lock(mtx);
            if(!renderer.data.editorMode) {
                renderer.updateCameraBuffer(mainCamera);
            } else {
                renderer.updateCameraBuffer(editorCamera);
            }
            scene.pushUpdatedModels();
            inputHandler.update();
            frameReady = true;
        }
        cv.notify_one();  // Notify GPU thread
        // Wait for drawFrame() to finish
        //std::unique_lock<std::mutex> lock(mtx);
        //cv.wait(lock, [] { return !frameReady; });
        }
    };
    auto gpuThread = [&](){
        while (!glfwWindowShouldClose(renderer.init.window)) {
            //Timer timer("rendering thread");
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return frameReady; });  // Wait until frame is ready

            renderer.drawFrame();  // Draw frame on GPU

            frameReady = false;  // Reset flag
            lock.unlock();
            cv.notify_one();
        }
    };

    std::thread cpu(cpuThread);
    std::thread gpu(gpuThread);
    // Ensure proper thread cleanup
    cpu.join(); 
    gpu.join();

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
