#include "Editor/Editor.h"
#include "GLFW/glfw3.h"
#include "SoulShard.h"
#include "Vulkan/VkRenderer.h"
#include <iostream>
#include <chrono>
int SoulShard::run() {
    VkRenderer::Init init;
    VkRenderer::RenderData render_data {
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

    
    
    auto lastTime = glfwGetTime(); 
    render_data.gui = ImguiModule();
    render_data.gui.init(&init, &render_data);
    bool keyAvailable = true;
    while (!glfwWindowShouldClose(init.window)) {
        glfwPollEvents();
        if (keyAvailable && glfwGetKey(init.window, GLFW_KEY_U) == GLFW_PRESS) {
            render_data.editorMode = !render_data.editorMode;
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
        float deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;
        if (!render_data.editorMode) {
            renderingResolution[0] = init.swapchain.extent.width;
            renderingResolution[1] = init.swapchain.extent.height;
        } else {
            renderingResolution[0] = render_data.gui.previewSize.x; 
            renderingResolution[1] = render_data.gui.previewSize.y;
        }


        for(auto & func : systems) {
            func(deltaTime);
        }
        VkRenderer::updateCameraBuffer(init, render_data, mainCamera);

        int res = draw_frame(init, render_data);
        if (res != 0) {
            std::cout << "failed to draw frame \n";
            return -1;
        }
    }
    init.disp.deviceWaitIdle();

    render_data.gui.destroy(init.device);
    VkRenderer::cleanup(init, render_data);
    return 0;
}
