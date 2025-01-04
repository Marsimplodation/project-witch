#include "VkRenderer.h"
#include "glm/fwd.hpp"
#include <iostream>
#include <vector>
namespace VkRenderer{
    int create_command_pool(Init& init, RenderData& data) {
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
    
    void TransitionImageLayout(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkPipelineStageFlags srcStage,
        VkPipelineStageFlags dstStage) {
        
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        } else if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = 0;
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            srcStage, dstStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
    }

    void renderModels(Init & init, RenderData & data, int i) {
        std::vector<glm::mat4> matrices;
        for (auto & model : data.models) {
            for(int i = 0; i < model.instanceCount; ++i){
                matrices.push_back(model.modelMatrices[i]);
            }
        }
        u32 modelIndex = 0;
        updateModelBuffer(init, data, matrices);
        init.disp.cmdBindIndexBuffer(data.command_buffers[i], data.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        for (auto & model : data.models) {
            vkCmdPushConstants(data.command_buffers[i],data.pipeline_layout,VK_SHADER_STAGE_VERTEX_BIT,
                0,                  // Offset
                sizeof(uint32_t),   // Size
                &modelIndex
            );

            init.disp.cmdDrawIndexed(data.command_buffers[i], model.triangleCount * 3, model.instanceCount, model.indexOffset, 0,0);
            modelIndex += model.instanceCount;
        }
    }

    void scene_offscreen_rendering(Init & init, RenderData & data, int i){
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

        VkDeviceSize offsets[] = {0};
        VkBuffer vertexBuffers[] = {data.vertexBuffer};
        init.disp.cmdBindVertexBuffers(data.command_buffers[i], 0, 1, vertexBuffers, offsets);
        init.disp.cmdBindDescriptorSets(data.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipeline_layout, 0, 1, &data.descriptorSets[data.current_frame], 0, nullptr);
        update_descriptor_sets(init,data);
        renderModels(init, data, i);
        init.disp.cmdEndRenderPass(data.command_buffers[i]);

    }
    void ui_onscreen_rendering(Init & init, RenderData & data, int i){
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

    void scene_onscreen_rendering(Init & init, RenderData & data, int i){
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

        VkDeviceSize offsets[] = {0};
        VkBuffer vertexBuffers[] = {data.vertexBuffer};
        init.disp.cmdBindVertexBuffers(data.command_buffers[i], 0, 1, vertexBuffers, offsets);
        init.disp.cmdBindDescriptorSets(data.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipeline_layout, 0, 1, &data.descriptorSets[data.current_frame], 0, nullptr);
        update_descriptor_sets(init,data);
        renderModels(init, data, i);
        init.disp.cmdEndRenderPass(data.command_buffers[i]);
    }
    
    int record_command_buffer(Init& init, RenderData& data, int i) {
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (init.disp.beginCommandBuffer(data.command_buffers[i], &begin_info) != VK_SUCCESS) {
            return -1; // failed to begin recording command buffer
        }
        if(data.editorMode){
            scene_offscreen_rendering(init, data, i);
            ui_onscreen_rendering(init, data, i);
        } else {
            scene_onscreen_rendering(init, data, i);
        }

        if (init.disp.endCommandBuffer(data.command_buffers[i]) != VK_SUCCESS) {
            std::cout << "failed to record command buffer\n";
            return -1; // failed to record command buffer!
        }
        return 0;

    }

    int create_command_buffers(Init& init, RenderData& data) {
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
}
