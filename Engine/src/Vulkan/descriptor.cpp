#include "vkRenderer.h"
#include <vulkan/vulkan_core.h>
namespace VkRenderer{
    int create_descriptor_pool(Init& init, RenderData& data) {
        VkDescriptorPoolSize poolSizes[1] = {};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSizes[0].descriptorCount = 2; // Change to 3 for the 3 descriptors (raygen, miss, hit)
        
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = poolSizes;
        poolInfo.maxSets = 3;  // Only need 1 descriptor set (if you're allocating 1 per frame)

        VkResult result = init.disp.createDescriptorPool(&poolInfo, nullptr, &data.descriptorPool); 
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool");
        }
        return 0;
    }

    int create_descriptor_layout(Init& init, RenderData& data) {
        VkDescriptorSetLayoutBinding vertexBinding = {};
        vertexBinding.binding = 0;
        vertexBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        vertexBinding.descriptorCount = 1;
        vertexBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  // This buffer is used by the raygen shader.


        VkDescriptorSetLayoutBinding indexBinding = {};
        indexBinding.binding = 1;
        indexBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        indexBinding.descriptorCount = 1;
        indexBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  // This buffer is used by the closest hit shader.
        

        VkDescriptorSetLayoutBinding bindings[] = {vertexBinding, indexBinding};

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = 2;  // Number of bindings
        layoutCreateInfo.pBindings = bindings;
        VkResult result = init.disp.createDescriptorSetLayout(&layoutCreateInfo, nullptr, &data.descriptorLayout);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        return 0;
    }
    
    int update_descriptor_sets(Init& init, RenderData& data) {
        VkDescriptorBufferInfo vertexBufferInfo = {};
        vertexBufferInfo.buffer = data.vertexBuffer;
        vertexBufferInfo.offset = 0;
        vertexBufferInfo.range = VK_WHOLE_SIZE;


        VkDescriptorBufferInfo indexBufferInfo = {};
        indexBufferInfo.buffer = data.indexBuffer;
        indexBufferInfo.offset = 0;
        indexBufferInfo.range = VK_WHOLE_SIZE;
        

        /*
        typedef struct VkWriteDescriptorSet {
            VkStructureType                  sType;
            const void*                      pNext;
            VkDescriptorSet                  dstSet;
            uint32_t                         dstBinding;
            uint32_t                         dstArrayElement;
            uint32_t                         descriptorCount;
            VkDescriptorType                 descriptorType;
            const VkDescriptorImageInfo*     pImageInfo;
            const VkDescriptorBufferInfo*    pBufferInfo;
            const VkBufferView*              pTexelBufferView;
        } VkWriteDescriptorSet;*/
        VkWriteDescriptorSet writeDescriptorSets[] = {
            { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, data.descriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &vertexBufferInfo, nullptr },
            { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, data.descriptorSet, 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &indexBufferInfo, nullptr },
        };

        init.disp.updateDescriptorSets(2,writeDescriptorSets, 0,nullptr);
        return 0;
    }
}
