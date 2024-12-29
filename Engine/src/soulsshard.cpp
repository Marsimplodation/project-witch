#include "GLFW/glfw3.h"
#include "SoulShard.h"
#include "Vulkan/vkRenderer.h"
#include <iostream>
#include <chrono>
int SoulShard::run() {
    VkRenderer::Init init;
    VkRenderer::RenderData render_data {
        .vertices = &vertices, 
        .indices = &indices,
        .vertShaderPath = "./vert.spv",
        .fragShaderPath = "./frag.spv",
    };

    if (0 != VkRenderer::device_initialization(init)) return -1;
    if (0 != VkRenderer::create_swapchain(init)) return -1;
    if (0 != VkRenderer::get_queues(init, render_data)) return -1;
    if (0 != VkRenderer::create_render_pass(init, render_data)) return -1;
    if (0 != VkRenderer::create_graphics_pipeline(init, render_data)) return -1;
    if (0 != VkRenderer::create_framebuffers(init, render_data)) return -1;
    if (0 != VkRenderer::create_command_pool(init, render_data)) return -1;
    if (0 != VkRenderer::createGeometryBuffers(init, render_data)) return -1;
    if (0 != VkRenderer::create_command_buffers(init, render_data)) return -1;
    if (0 != VkRenderer::create_sync_objects(init, render_data)) return -1;

    
    
    auto lastTime = glfwGetTime(); 
    while (!glfwWindowShouldClose(init.window)) {
        glfwPollEvents();
        auto view = entities.view<Model>();
        render_data.models.clear();
        for (auto entity : view) {
            auto& model = view.get<Model>(entity);
            render_data.models.push_back(model);
        }
        auto currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        for(auto & func : systems) {
            func(deltaTime);
        }

        int res = draw_frame(init, render_data);
        if (res != 0) {
            std::cout << "failed to draw frame \n";
            return -1;
        }
    }
    init.disp.deviceWaitIdle();

    VkRenderer::cleanup(init, render_data);
    return 0;
}
