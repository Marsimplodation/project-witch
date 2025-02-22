#include "SoulShard.h"
#include "VkRenderer.h"
#include "glm/fwd.hpp"
#include <iostream>
#include <vector>
int VkRenderer::create_command_pool() {
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = init.device.get_queue_index(vkb::QueueType::graphics).value();

    if (init.disp.createCommandPool(&pool_info, nullptr, &data.command_pool) != VK_SUCCESS) {
        std::cout << "failed to create command pool\n";
        return -1; // failed to create command pool
    }
    return 0;
}


void VkRenderer::renderModels(int i) {
    u32 modelIndex = 0;
    SoulShard & engine = *((SoulShard*)enginePtr);

    for (auto & model : engine.scene.linearModels) {
        vkCmdPushConstants(data.command_buffers[i],data.pipeline_layout,VK_SHADER_STAGE_VERTEX_BIT,
            0,                  // Offset
            sizeof(uint32_t),   // Size
            &modelIndex
        );

        init.disp.cmdDrawIndexed(data.command_buffers[i], model.triangleCount * 3, model.instanceCount, model.indexOffset, 0,0);
        modelIndex += model.instanceCount;
    }
}

void VkRenderer::scene_shadow_rendering(int i){
    for (int c = 0; c < SHADOW_CASCADES; ++c) {
        VkRenderPassBeginInfo offsceen_pass_info = {};
        offsceen_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        offsceen_pass_info.renderPass = data.shadow_pass;
        offsceen_pass_info.framebuffer = data.shadow_framebuffers[c];
        offsceen_pass_info.renderArea.offset = { 0, 0 };
        VkExtent2D extent;
        extent.width = SHADOW_MAP_RES; 
        extent.height = SHADOW_MAP_RES; 
        offsceen_pass_info.renderArea.extent = extent; 
        std::array<VkClearValue, 1> clearValues{};
        clearValues[0].depthStencil = {1.0f, 0};
        offsceen_pass_info.clearValueCount = 1;
        offsceen_pass_info.pClearValues = clearValues.data();

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = SHADOW_MAP_RES; 
        viewport.height = SHADOW_MAP_RES;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = extent;
        init.disp.cmdSetViewport(data.command_buffers[i], 0, 1, &viewport);
        init.disp.cmdSetScissor(data.command_buffers[i], 0, 1, &scissor);

        init.disp.cmdBeginRenderPass(data.command_buffers[i], &offsceen_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        init.disp.cmdBindPipeline(data.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.shadow_pipeline);

        vkCmdPushConstants(data.command_buffers[i],data.pipeline_layout,VK_SHADER_STAGE_VERTEX_BIT,
            4,                  // Offset
            sizeof(uint32_t),   // Size
            &c
        );
        renderModels(i);
        init.disp.cmdEndRenderPass(data.command_buffers[i]);
    }
}

void VkRenderer::scene_offscreen_rendering(int i){
    VkRenderPassBeginInfo offsceen_pass_info = {};
    offsceen_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    offsceen_pass_info.renderPass = data.offscreen_pass;
    offsceen_pass_info.framebuffer = data.offscreen_framebuffers[i];
    offsceen_pass_info.renderArea.offset = { 0, 0 };
    VkExtent2D extent;
    extent.width = std::min((u32)data.gui.previewSize.x, init.swapchain.extent.width); 
    extent.height = std::min((u32)data.gui.previewSize.y, init.swapchain.extent.height); 
    offsceen_pass_info.renderArea.extent = extent; 
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    offsceen_pass_info.clearValueCount = 2;
    offsceen_pass_info.pClearValues = clearValues.data();

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = init.swapchain.extent;

    init.disp.cmdSetViewport(data.command_buffers[i], 0, 1, &viewport);
    init.disp.cmdSetScissor(data.command_buffers[i], 0, 1, &scissor);

    init.disp.cmdBeginRenderPass(data.command_buffers[i], &offsceen_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    init.disp.cmdBindPipeline(data.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.offscreen_pipeline);

    renderModels(i);
    init.disp.cmdEndRenderPass(data.command_buffers[i]);

}
void VkRenderer::ui_onscreen_rendering(int i){
    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = data.render_pass;
    render_pass_info.framebuffer = data.framebuffers[i];
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = init.swapchain.extent;
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    render_pass_info.clearValueCount = 2;
    render_pass_info.pClearValues = clearValues.data();

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)init.swapchain.extent.width;
    viewport.height = (float)init.swapchain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = init.swapchain.extent;

    init.disp.cmdSetViewport(data.command_buffers[i], 0, 1, &viewport);
    init.disp.cmdSetScissor(data.command_buffers[i], 0, 1, &scissor);

    init.disp.cmdBeginRenderPass(data.command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    data.gui.update(&init, &data);
    init.disp.cmdBindPipeline(data.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.graphics_pipeline);

    init.disp.cmdEndRenderPass(data.command_buffers[i]);
}

void VkRenderer::scene_onscreen_rendering(int i){
    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = data.render_pass;
    render_pass_info.framebuffer = data.framebuffers[i];
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = init.swapchain.extent;
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    render_pass_info.clearValueCount = 2;
    render_pass_info.pClearValues = clearValues.data();

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)init.swapchain.extent.width;
    viewport.height = (float)init.swapchain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = init.swapchain.extent;

    init.disp.cmdSetViewport(data.command_buffers[i], 0, 1, &viewport);
    init.disp.cmdSetScissor(data.command_buffers[i], 0, 1, &scissor);

    init.disp.cmdBeginRenderPass(data.command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    init.disp.cmdBindPipeline(data.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.graphics_pipeline);
    renderModels(i);
    init.disp.cmdEndRenderPass(data.command_buffers[i]);
}

int VkRenderer::record_command_buffer(int i) {
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (init.disp.beginCommandBuffer(data.command_buffers[i], &begin_info) != VK_SUCCESS) {
        return -1; // failed to begin recording command buffer
    }
    SoulShard & engine = *((SoulShard*)enginePtr);
    updateModelBuffer(engine.scene.modelMatrices);
    engine.scene.updateLights();
    updateLightBuffer(engine.scene.sceneLight);

    init.disp.cmdBindIndexBuffer(data.command_buffers[i], data.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    VkDeviceSize offsets[] = {0};
    VkBuffer vertexBuffers[] = {data.vertexBuffer};
    init.disp.cmdBindVertexBuffers(data.command_buffers[i], 0, 1, vertexBuffers, offsets);

    update_descriptor_sets();
    init.disp.cmdBindDescriptorSets(data.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipeline_layout, 0, 1, &data.descriptorSets[data.current_frame], 0, nullptr);

    scene_shadow_rendering(i);

    if(data.editorMode){
        scene_offscreen_rendering(i);
        ui_onscreen_rendering(i);
    } else {
        scene_onscreen_rendering(i);
    }

    if (init.disp.endCommandBuffer(data.command_buffers[i]) != VK_SUCCESS) {
        std::cout << "failed to record command buffer\n";
        return -1; // failed to record command buffer!
    }
    return 0;

}

int VkRenderer::create_command_buffers() {
    data.command_buffers.resize(data.framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = data.command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)data.command_buffers.size();

    if (init.disp.allocateCommandBuffers(&allocInfo, data.command_buffers.data()) != VK_SUCCESS) {
        return -1; // failed to allocate command buffers;
    }

    for (size_t i = 0; i < data.command_buffers.size(); i++) {
    }
    return 0;
}
