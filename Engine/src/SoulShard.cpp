#include "Editor/Editor.h"
#include "GLFW/glfw3.h"
#include "SoulShard.h"
#include "InputHandling/InputHandling.h"
#include "Vulkan/VkRenderer.h"
#include <cstdlib>
#include <iostream>
#include <chrono>
VkRenderer::Init init;
VkRenderer::RenderData render_data;
int SoulShard::run() {
    render_data = VkRenderer::RenderData {
        .vertices = &gpuGeometry.vertices, 
        .indices = &gpuGeometry.indices,
        .vertShaderPath = "./vert.spv",
        .fragShaderPath = "./frag.spv",
    };

    if (0 != VkRenderer::device_initialization(init)) return -1;
    if (0 != VkRenderer::create_swapchain(init)) return -1;
    if (0 != VkRenderer::get_queues(init, render_data)) return -1;
    if (0 != VkRenderer::create_render_pass(init, render_data)) return -1;
    if (0 != VkRenderer::create_off_screen_render_pass(init, render_data)) return -1;
    if (0 != VkRenderer::create_descriptor_pool(init, render_data)) return -1;
    if (0 != VkRenderer::create_descriptor_layout(init, render_data)) return -1;
    if (0 != VkRenderer::create_graphics_pipeline(init, render_data)) return -1;
    if (0 != VkRenderer::create_framebuffers(init, render_data)) return -1;
    if (0 != VkRenderer::create_command_pool(init, render_data)) return -1;
    if (0 != VkRenderer::createGeometryBuffers(init, render_data)) return -1;
    if (0 != VkRenderer::createUniformBuffers(init, render_data)) return -1;

    
    if (0 != VkRenderer::create_command_buffers(init, render_data)) return -1;
    if (0 != VkRenderer::create_sync_objects(init, render_data)) return -1;
    inputHandler.init(init.window);

    
    
    auto lastTime = glfwGetTime(); 
    render_data.gui = ImguiModule();
    render_data.gui.init(&init, &render_data);
    render_data.gui.enginePtr = this;
    bool keyAvailable = true;
    while (!glfwWindowShouldClose(init.window)) {
        glfwPollEvents();
        if (keyAvailable && glfwGetKey(init.window, GLFW_KEY_U) == GLFW_PRESS) {
            render_data.editorMode = !render_data.editorMode;
            inputHandler.releaseMouse();
            keyAvailable = false;
        }
        if (glfwGetKey(init.window, GLFW_KEY_U) == GLFW_RELEASE) {
            keyAvailable = true;
        }
        auto view = entities.view<Model>();
        render_data.models.clear();
        for (auto entity : view) {
            auto& model = view.get<Model>(entity);
            render_data.models.push_back(model);
        }
        auto currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;
        if (!render_data.editorMode) {
            renderingResolution[0] = init.swapchain.extent.width;
            renderingResolution[1] = init.swapchain.extent.height;
        } else {
            renderingResolution[0] = render_data.gui.previewSize.x; 
            renderingResolution[1] = render_data.gui.previewSize.y;
        }

        if(!render_data.editorMode) {
            for(auto & system : systems) {
                if(system.active)system.func(deltaTime);
            }
            VkRenderer::updateCameraBuffer(init, render_data, mainCamera);
        } else {
            for(auto & system : systems) {
                if(system.active)system.func(deltaTime);
            }
            VkRenderer::updateCameraBuffer(init, render_data, editorCamera);
        }

        int res = draw_frame(init, render_data);
        if (res != 0) {
            std::cout << "failed to draw frame \n";
            return -1;
        }
        inputHandler.update();
    }
    init.disp.deviceWaitIdle();

    render_data.gui.destroy(init.device);
    VkRenderer::cleanup(init, render_data);
    return 0;
}

void SoulShard::registerSystem(void(*func)(float deltaTime), const char * name) {
    systems.push_back({.active = true,
        .func = func,
        .name = std::string(name)
    });
}
