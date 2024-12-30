#include "VkRenderer.h"
#include "glm/fwd.hpp"
#include <glm/fwd.hpp>
#include <utility>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>


namespace VkRenderer{
//--- creating buffers----//
//  
    u32 findMemoryType(Init& init, u32 typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        init.inst_disp.getPhysicalDeviceMemoryProperties(init.physicalDevice, &memoryProperties); 

        // Loop through all available memory types and find a suitable one
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            // Check if the type is available in the filter and supports the required properties
            if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;  // Return the index of the suitable memory type
            }
        }

        // If no suitable memory type is found, we return an invalid index
        throw std::runtime_error("Failed to find suitable memory type!");
    }
    
    VkResult createBuffer(Init& init, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
        // Create the buffer
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (init.disp.createBuffer(&bufferInfo, nullptr, buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer!");
        }

        // Get memory requirements for the buffer
        VkMemoryRequirements memRequirements;
        init.disp.getBufferMemoryRequirements(*buffer, &memRequirements);

        // Allocate memory
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(init, memRequirements.memoryTypeBits, properties);

        if (init.disp.allocateMemory(&allocInfo, nullptr, bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate buffer memory!");
        }

        init.disp.bindBufferMemory(*buffer, *bufferMemory, 0);

        return VK_SUCCESS;
    }
    void copyDataToBuffer(Init & init, VkDeviceMemory buffer, const void* data, VkDeviceSize bufferSize) {
        void* mappedMemory;
        init.disp.mapMemory(buffer, 0, bufferSize, 0, &mappedMemory);
        memcpy(mappedMemory, data, static_cast<size_t>(bufferSize));
        init.disp.unmapMemory(buffer);
    }

    void copyDataToBufferWithStaging(Init & init, RenderData & render_data, VkBuffer buffer, const void* data, VkDeviceSize bufferSize) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(init, bufferSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &stagingBuffer, &stagingBufferMemory);

        void* mappedMemory;
        init.disp.mapMemory(stagingBufferMemory, 0, bufferSize, 0, &mappedMemory);
        memcpy(mappedMemory, data, static_cast<size_t>(bufferSize));
        init.disp.unmapMemory(stagingBufferMemory);

        //copy data to actual buffer
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = render_data.command_pool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        init.disp.allocateCommandBuffers(&allocInfo, &commandBuffer);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        init.disp.beginCommandBuffer(commandBuffer, &beginInfo);
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = bufferSize;
        init.disp.cmdCopyBuffer(commandBuffer, stagingBuffer, buffer, 1, &copyRegion);
        init.disp.endCommandBuffer(commandBuffer);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        init.disp.queueSubmit(render_data.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
        init.disp.queueWaitIdle(render_data.graphics_queue);
        init.disp.destroyBuffer(stagingBuffer, nullptr); 
        init.disp.freeMemory(stagingBufferMemory, nullptr); 
    }

    int createGeometryBuffers(Init& init, RenderData& data) {
        createBuffer(init, sizeof(Vertex) * data.vertices->size(),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &data.vertexBuffer,
                 &data.vertexBufferMemory);
        copyDataToBufferWithStaging(init,data,data.vertexBuffer, data.vertices->data(), sizeof(Vertex)*data.vertices->size());
        
        createBuffer(init, sizeof(u32) * data.indices->size(),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &data.indexBuffer,
                 &data.indexBufferMemory);
        copyDataToBufferWithStaging(init,data,data.indexBuffer, data.indices->data(), sizeof(u32)*data.indices->size());
        return 0;
    }
    
    int createUniformBuffers(Init& init, RenderData& data) {
        VkBuffer cameraBuffer;
        VkDeviceMemory cameraMemory;
        createBuffer(init, sizeof(Camera),
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &cameraBuffer,
                 &cameraMemory);
        data.uniformBuffers.push_back(std::pair<VkBuffer, VkDeviceMemory>(cameraBuffer, cameraMemory));
        
        VkBuffer modelBuffer;
        VkDeviceMemory modelMemory;
        createBuffer(init, sizeof(glm::mat4) * 100,
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &modelBuffer,
                 &modelMemory);
        data.uniformBuffers.push_back(std::pair<VkBuffer, VkDeviceMemory>(modelBuffer, modelMemory));
        return 0;
    }

    void updateCameraBuffer(Init& init, RenderData& data, Camera & camera) {
        copyDataToBuffer(init, data.uniformBuffers[0].second, &camera, sizeof(Camera)); 
    }
    
    void updateModelBuffer(Init& init, RenderData& data, Model & model) {
        copyDataToBuffer(init, data.uniformBuffers[1].second, model.modelMatrices.data(), sizeof(glm::mat4) * model.instanceCount); 
    }

    void destroyBuffers(Init& init, RenderData& data) {
        init.disp.destroyBuffer(data.vertexBuffer, nullptr);
        init.disp.destroyBuffer(data.indexBuffer, nullptr);
        init.disp.freeMemory(data.vertexBufferMemory, nullptr);
        init.disp.freeMemory(data.indexBufferMemory, nullptr);
        for (auto & buffer : data.uniformBuffers) {
            init.disp.destroyBuffer(buffer.first, nullptr);
            init.disp.freeMemory(buffer.second, nullptr);
        }
    } 
}
