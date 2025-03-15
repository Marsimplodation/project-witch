#include "SoulShard.h"
#include "VkRenderer.h"
#include "glm/fwd.hpp"
#include "types/defines.h"
#include "types/types.h"
#include <glm/fwd.hpp>
#include <utility>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>


//--- creating buffers----//
//  
u32 VkRenderer::findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    init.instDisp.getPhysicalDeviceMemoryProperties(init.physicalDevice, &memoryProperties); 

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

VkResult VkRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
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
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (init.disp.allocateMemory(&allocInfo, nullptr, bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    init.disp.bindBufferMemory(*buffer, *bufferMemory, 0);

    return VK_SUCCESS;
}
void VkRenderer::copyDataToBuffer(VkDeviceMemory buffer, const void* data, VkDeviceSize bufferSize) {
    void* mappedMemory;
    init.disp.mapMemory(buffer, 0, bufferSize, 0, &mappedMemory);
    memcpy(mappedMemory, data, static_cast<size_t>(bufferSize));
    init.disp.unmapMemory(buffer);
}

void VkRenderer::copyDataToBufferWithStaging(VkBuffer buffer, const void* bufferData, VkDeviceSize bufferSize) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &stagingBuffer, &stagingBufferMemory);

    void* mappedMemory;
    init.disp.mapMemory(stagingBufferMemory, 0, bufferSize, 0, &mappedMemory);
    memcpy(mappedMemory, bufferData, static_cast<size_t>(bufferSize));
    init.disp.unmapMemory(stagingBufferMemory);

    //copy data to actual buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = data.commandPool;
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

    init.disp.queueSubmit(data.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    init.disp.queueWaitIdle(data.graphicsQueue);
    init.disp.destroyBuffer(stagingBuffer, nullptr); 
    init.disp.freeMemory(stagingBufferMemory, nullptr); 
}

int VkRenderer::createGeometryBuffers() {
    createBuffer(sizeof(Vertex) * data.vertices->size(),
             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
             &data.vertexBuffer,
             &data.vertexBufferMemory);
    copyDataToBufferWithStaging(data.vertexBuffer, data.vertices->data(), sizeof(Vertex)*data.vertices->size());
    
    createBuffer(sizeof(u32) * data.indices->size(),
             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
             &data.indexBuffer,
             &data.indexBufferMemory);
    copyDataToBufferWithStaging(data.indexBuffer, data.indices->data(), sizeof(u32)*data.indices->size());

        // Create Indirect Draw Buffer
    size_t drawCmdBufferSize = sizeof(VkDrawIndexedIndirectCommand) * MAX_DRAWS;
    u32 count = data.swapchainImages.size();
    for(int idx = 0; idx < SHADOW_CASCADES + 1; idx++) {
        data.indirectDrawBufferMemorys[idx].resize(count);
        data.indirectDrawBuffers[idx].resize(count);
        data.indirectDrawCounts[idx].resize(count);
        for(int i = 0; i < count; ++i){
        createBuffer(drawCmdBufferSize,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     &data.indirectDrawBuffers[idx][i],
                     &data.indirectDrawBufferMemorys[idx][i]);
        }
    }

    return 0;
}


void VkRenderer::updateIndirectDrawBuffer(int renderingIndex) {
    SoulShard & engine = *((SoulShard*)enginePtr);
    
    std::vector<VkDrawIndexedIndirectCommand> drawCommands;
    auto offset = engine.scene.matrixOffsets[renderingIndex];
    u32 modelIndex = offset;

    for (auto &model : engine.scene.linearModels[renderingIndex]) {
        if (model.instanceCount == 0) continue;

        VkDrawIndexedIndirectCommand drawCmd = {};
        drawCmd.indexCount = model.triangleCount * 3;
        drawCmd.instanceCount = model.instanceCount;
        drawCmd.firstIndex = model.indexOffset;
        drawCmd.vertexOffset = 0;
        drawCmd.firstInstance = modelIndex;  // Pass the model index as firstInstance

        drawCommands.push_back(drawCmd);
        modelIndex += model.instanceCount;
    }

    copyDataToBuffer(data.indirectDrawBufferMemorys[renderingIndex][data.currentImgIndex], drawCommands.data(), sizeof(VkDrawIndexedIndirectCommand) * drawCommands.size());
    data.indirectDrawCounts[renderingIndex][data.currentImgIndex] = drawCommands.size();
}

int VkRenderer::createUniformBuffers() {
    VkBuffer cameraBuffer;
    VkDeviceMemory cameraMemory;
    createBuffer(sizeof(Camera),
             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
             &cameraBuffer,
             &cameraMemory);
    data.uniformBuffers.push_back(std::pair<VkBuffer, VkDeviceMemory>(cameraBuffer, cameraMemory));
    
    VkBuffer modelBuffer;
    VkDeviceMemory modelMemory;
    createBuffer(sizeof(glm::mat4) * MAX_INSTANCES * (SHADOW_CASCADES + 1),
             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
             &modelBuffer,
             &modelMemory);
    data.uniformBuffers.push_back(std::pair<VkBuffer, VkDeviceMemory>(modelBuffer, modelMemory));
    
    VkBuffer lightBuffer;
    VkDeviceMemory lightMemory;
    createBuffer(sizeof(DirectionLight),
             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
             &lightBuffer,
             &lightMemory);
    data.uniformBuffers.push_back(std::pair<VkBuffer, VkDeviceMemory>(lightBuffer, lightMemory));
    VkBuffer materialBuffer;
    VkDeviceMemory materialMemory;
    createBuffer(sizeof(Material) * MAX_MATERIALS,
             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
             &materialBuffer,
             &materialMemory);
    data.uniformBuffers.push_back(std::pair<VkBuffer, VkDeviceMemory>(materialBuffer, materialMemory));
    return 0;
}

void VkRenderer::updateCameraBuffer(Camera & camera) {
    copyDataToBuffer(data.uniformBuffers[0].second, &camera, sizeof(Camera)); 
}

void VkRenderer::updateModelBuffer(std::vector<glm::mat4> & matrices) {
    if(matrices.size() == 0) return;
    copyDataToBuffer(data.uniformBuffers[1].second, matrices.data(), sizeof(glm::mat4) * matrices.size()); 
}

void VkRenderer::updateLightBuffer(DirectionLight & light) {
    copyDataToBuffer(data.uniformBuffers[2].second, &light, sizeof(DirectionLight)); 
}

void VkRenderer::updatematerialBuffer() {
    copyDataToBuffer(data.uniformBuffers[3].second, data.materials.data(), sizeof(Material) * MAX_MATERIALS); 
}

void VkRenderer::destroyBuffers() {
    init.disp.destroyBuffer(data.vertexBuffer, nullptr);
    init.disp.destroyBuffer(data.indexBuffer, nullptr);
    init.disp.freeMemory(data.vertexBufferMemory, nullptr);
    init.disp.freeMemory(data.indexBufferMemory, nullptr);
    for (auto & buffer : data.uniformBuffers) {
        init.disp.destroyBuffer(buffer.first, nullptr);
        init.disp.freeMemory(buffer.second, nullptr);
    }
    for (size_t idx = 0; idx < SHADOW_CASCADES + 1; idx++) {
        for (size_t i = 0; i < data.indirectDrawBuffers[idx].size(); i++) {
            init.disp.destroyBuffer(data.indirectDrawBuffers[idx][i], nullptr);
            init.disp.freeMemory(data.indirectDrawBufferMemorys[idx][i], nullptr);
        }
    }
} 
