#include "SoulShard.h"
#include "VkRenderer.h"
#include "glm/fwd.hpp"
#include <iostream>
#include <vector>
int VkRenderer::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = init.device.get_queue_index(vkb::QueueType::graphics).value();

    if (init.disp.createCommandPool(&poolInfo, nullptr, &data.commandPool) != VK_SUCCESS) {
        std::cout << "failed to create command pool\n";
        return -1; // failed to create command pool
    }
    return 0;
}


void VkRenderer::renderModels(int i, int renderingIndex) {
    SoulShard & engine = *((SoulShard*)enginePtr);
    auto offset = engine.scene.matrixOffsets[renderingIndex];
    u32 modelIndex = offset;


    for (auto & model : engine.scene.linearModels[renderingIndex]) {
        vkCmdPushConstants(data.commandBuffers[i],data.pipelineLayout,VK_SHADER_STAGE_VERTEX_BIT,
            0,                  // Offset
            sizeof(uint32_t),   // Size
            &modelIndex
        );
        
        if(model.instanceCount == 0) continue;
        init.disp.cmdDrawIndexed(data.commandBuffers[i], model.triangleCount * 3, model.instanceCount, model.indexOffset, 0,0);
        modelIndex += model.instanceCount;
        data.drawCalls++;
        data.instancesRendered += model.instanceCount;
    }
}

void VkRenderer::sceneShadowRendering(int i){
    for (int c = 0; c < SHADOW_CASCADES; ++c) {
        VkRenderPassBeginInfo offsceenPassInfo = {};
        offsceenPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        offsceenPassInfo.renderPass = data.shadowPass;
        offsceenPassInfo.framebuffer = data.shadowFramebuffers[i][c];
        offsceenPassInfo.renderArea.offset = { 0, 0 };
        VkExtent2D extent;
        extent.width = SHADOW_MAP_RES[c]; 
        extent.height = SHADOW_MAP_RES[c]; 
        offsceenPassInfo.renderArea.extent = extent; 
        std::array<VkClearValue, 1> clearValues{};
        clearValues[0].depthStencil = {1.0f, 0};
        offsceenPassInfo.clearValueCount = 1;
        offsceenPassInfo.pClearValues = clearValues.data();

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = SHADOW_MAP_RES[c]; 
        viewport.height = SHADOW_MAP_RES[c];
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = extent;
        init.disp.cmdSetViewport(data.commandBuffers[i], 0, 1, &viewport);
        init.disp.cmdSetScissor(data.commandBuffers[i], 0, 1, &scissor);

        init.disp.cmdBeginRenderPass(data.commandBuffers[i], &offsceenPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        init.disp.cmdBindPipeline(data.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.shadowPipeline);

        vkCmdPushConstants(data.commandBuffers[i],data.shadowPipelineLayout,VK_SHADER_STAGE_VERTEX_BIT,
            4,                  // Offset
            sizeof(u32),   // Size
            &c
        );
        renderModels(i, 1+c);
        init.disp.cmdEndRenderPass(data.commandBuffers[i]);
    }
}

void VkRenderer::sceneOffscreenRendering(int i){
    VkRenderPassBeginInfo offsceenPassInfo = {};
    offsceenPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    offsceenPassInfo.renderPass = data.offscreenPass;
    offsceenPassInfo.framebuffer = data.offscreenFramebuffers[i];
    offsceenPassInfo.renderArea.offset = { 0, 0 };
    VkExtent2D extent;
    extent.width = std::min((u32)data.gui.previewSize.x, init.swapchain.extent.width); 
    extent.height = std::min((u32)data.gui.previewSize.y, init.swapchain.extent.height); 
    offsceenPassInfo.renderArea.extent = extent; 
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    offsceenPassInfo.clearValueCount = 2;
    offsceenPassInfo.pClearValues = clearValues.data();

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

    init.disp.cmdSetViewport(data.commandBuffers[i], 0, 1, &viewport);
    init.disp.cmdSetScissor(data.commandBuffers[i], 0, 1, &scissor);

    init.disp.cmdBeginRenderPass(data.commandBuffers[i], &offsceenPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    init.disp.cmdBindPipeline(data.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.offscreenPipeline);

    renderModels(i, 0);
    init.disp.cmdEndRenderPass(data.commandBuffers[i]);

}
void VkRenderer::uiOnscreenRendering(int i){
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = data.renderPass;
    renderPassInfo.framebuffer = data.framebuffers[i];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = init.swapchain.extent;
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues.data();

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

    init.disp.cmdSetViewport(data.commandBuffers[i], 0, 1, &viewport);
    init.disp.cmdSetScissor(data.commandBuffers[i], 0, 1, &scissor);

    init.disp.cmdBeginRenderPass(data.commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    data.gui.update(&init, &data);
    init.disp.cmdBindPipeline(data.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.graphicsPipeline);

    init.disp.cmdEndRenderPass(data.commandBuffers[i]);
}

void VkRenderer::sceneOnscreenRendering(int i){
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = data.renderPass;
    renderPassInfo.framebuffer = data.framebuffers[i];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = init.swapchain.extent;
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues.data();

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

    init.disp.cmdSetViewport(data.commandBuffers[i], 0, 1, &viewport);
    init.disp.cmdSetScissor(data.commandBuffers[i], 0, 1, &scissor);

    init.disp.cmdBeginRenderPass(data.commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    init.disp.cmdBindPipeline(data.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.graphicsPipeline);
    renderModels(i, 0);
    init.disp.cmdEndRenderPass(data.commandBuffers[i]);
}

int VkRenderer::recordCommandBuffer(int i) {
    data.drawCalls = 0;
    data.instancesRendered = 0;
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (init.disp.beginCommandBuffer(data.commandBuffers[i], &beginInfo) != VK_SUCCESS) {
        return -1; // failed to begin recording command buffer
    }
    SoulShard & engine = *((SoulShard*)enginePtr);
    updateModelBuffer(engine.scene.modelMatrices);
    engine.scene.updateLights();
    updateLightBuffer(engine.scene.sceneLight);

    init.disp.cmdBindIndexBuffer(data.commandBuffers[i], data.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    VkDeviceSize offsets[] = {0};
    VkBuffer vertexBuffers[] = {data.vertexBuffer};
    init.disp.cmdBindVertexBuffers(data.commandBuffers[i], 0, 1, vertexBuffers, offsets);


    updateShadowDescriptorSets();
    init.disp.cmdBindDescriptorSets(data.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.shadowPipelineLayout, 0, 1, &data.descriptorShadowSets[data.currentFrame], 0, nullptr);
    if(engine.scene.sceneLight.castShadows)sceneShadowRendering(i);

    updateDescriptorSets();
    init.disp.cmdBindDescriptorSets(data.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipelineLayout, 0, 1, &data.descriptorSets[data.currentFrame], 0, nullptr);

    if(data.editorMode){
        sceneOffscreenRendering(i);
        uiOnscreenRendering(i);
    } else {
        sceneOnscreenRendering(i);
    }

    if (init.disp.endCommandBuffer(data.commandBuffers[i]) != VK_SUCCESS) {
        std::cout << "failed to record command buffer\n";
        return -1; // failed to record command buffer!
    }
    return 0;

}

int VkRenderer::createCommandBuffers() {
    data.commandBuffers.resize(data.framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = data.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (u32)data.commandBuffers.size();

    if (init.disp.allocateCommandBuffers(&allocInfo, data.commandBuffers.data()) != VK_SUCCESS) {
        return -1; // failed to allocate command buffers;
    }

    for (size_t i = 0; i < data.commandBuffers.size(); i++) {
    }
    return 0;
}
