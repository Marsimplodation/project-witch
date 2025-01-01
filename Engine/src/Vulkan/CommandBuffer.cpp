#include "VkRenderer.h"
#include <iostream>
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

    void scene_offscreen_rendering(Init & init, RenderData & data, int i){
        VkRenderPassBeginInfo offsceen_pass_info = {};
        offsceen_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        offsceen_pass_info.renderPass = data.offscreen_pass;
        offsceen_pass_info.framebuffer = data.offscreen_framebuffers[i];
        offsceen_pass_info.renderArea.offset = { 0, 0 };
        offsceen_pass_info.renderArea.extent = init.swapchain.extent;
        VkClearValue clearColor{ { { 0.0f, 0.0f, 0.0f, 1.0f } } };
        offsceen_pass_info.clearValueCount = 1;
        offsceen_pass_info.pClearValues = &clearColor;

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

        init.disp.cmdBeginRenderPass(data.command_buffers[i], &offsceen_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        init.disp.cmdBindPipeline(data.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.offscreen_pipeline);

        VkDeviceSize offsets[] = {0};
        VkBuffer vertexBuffers[] = {data.vertexBuffer};
        init.disp.cmdBindVertexBuffers(data.command_buffers[i], 0, 1, vertexBuffers, offsets);
        for (auto & model : data.models) {
            updateModelBuffer(init, data, model);
            update_descriptor_sets(init,data);
            init.disp.cmdBindDescriptorSets(data.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipeline_layout, 0, 1, &data.descriptorSets[data.current_frame], 0, nullptr);
            init.disp.cmdBindIndexBuffer(data.command_buffers[i], data.indexBuffer, model.indexOffset*sizeof(u32), VK_INDEX_TYPE_UINT32);
            init.disp.cmdDrawIndexed(data.command_buffers[i], model.triangleCount * 3, model.instanceCount, 0, 0,0);
        }

        init.disp.cmdEndRenderPass(data.command_buffers[i]);

    }
    void ui_onscreen_rendering(Init & init, RenderData & data, int i){
        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = data.render_pass;
        render_pass_info.framebuffer = data.framebuffers[i];
        render_pass_info.renderArea.offset = { 0, 0 };
        render_pass_info.renderArea.extent = init.swapchain.extent;
        VkClearValue clearColor{ { { 0.0f, 0.0f, 0.0f, 1.0f } } };
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clearColor;

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

        init.disp.cmdEndRenderPass(data.command_buffers[i]);
    }

    void scene_onscreen_rendering(Init & init, RenderData & data, int i){
        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = data.render_pass;
        render_pass_info.framebuffer = data.framebuffers[i];
        render_pass_info.renderArea.offset = { 0, 0 };
        render_pass_info.renderArea.extent = init.swapchain.extent;
        VkClearValue clearColor{ { { 0.0f, 0.0f, 0.0f, 1.0f } } };
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clearColor;

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
        for (auto & model : data.models) {
            updateModelBuffer(init, data, model);
            update_descriptor_sets(init,data);
            init.disp.cmdBindDescriptorSets(data.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipeline_layout, 0, 1, &data.descriptorSets[data.current_frame], 0, nullptr);
            init.disp.cmdBindIndexBuffer(data.command_buffers[i], data.indexBuffer, model.indexOffset*sizeof(u32), VK_INDEX_TYPE_UINT32);
            init.disp.cmdDrawIndexed(data.command_buffers[i], model.triangleCount * 3, model.instanceCount, 0, 0,0);
        }

        init.disp.cmdEndRenderPass(data.command_buffers[i]);
    }
    
    int record_command_buffer(Init& init, RenderData& data, int i) {
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (init.disp.beginCommandBuffer(data.command_buffers[i], &begin_info) != VK_SUCCESS) {
            return -1; // failed to begin recording command buffer
        }
        scene_offscreen_rendering(init, data, i);
        ui_onscreen_rendering(init, data, i);

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
            record_command_buffer(init, data, i);
        }
        return 0;
    }
}
