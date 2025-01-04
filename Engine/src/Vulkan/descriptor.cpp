#include "VkRenderer.h"
#include <vulkan/vulkan_core.h>
int VkRenderer::create_descriptor_pool() {
    VkDescriptorPoolSize poolSizes[2] = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 2 * MAX_FRAMES_IN_FLIGHT; // Change to 3 for the 3 descriptors (raygen, miss, hit)
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 100 * MAX_FRAMES_IN_FLIGHT; // Change to 3 for the 3 descriptors (raygen, miss, hit)
    
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;  // Only need 1 descriptor set (if you're allocating 1 per frame)

    //VkResult result = init.disp.createDescriptorPool(&poolInfo, nullptr, &data.descriptorPool); 
    VkResult result = init.disp.createDescriptorPool(&poolInfo, nullptr, &data.descriptorPool); 
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool");
    }

    return 0;
}

int VkRenderer::create_descriptor_layout() {
    VkDescriptorSetLayoutBinding cameraBinding = {};
    cameraBinding.binding = 0;
    cameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraBinding.descriptorCount = 1;
    cameraBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  // This buffer is used by the closest hit shader.
    
    VkDescriptorSetLayoutBinding modelBinding = {};
    modelBinding.binding = 1;
    modelBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    modelBinding.descriptorCount = 1;
    modelBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  // This buffer is used by the closest hit shader.

    VkDescriptorSetLayoutBinding textureLayoutBinding{};
    textureLayoutBinding.binding = 2;  // Binding index
    textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureLayoutBinding.descriptorCount = 100;
    textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    textureLayoutBinding.pImmutableSamplers = nullptr;  // No static samplers

    

    VkDescriptorSetLayoutBinding bindings[] = {cameraBinding, modelBinding, textureLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 3;  // Number of bindings
    layoutCreateInfo.pBindings = bindings;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i){
        VkResult result = init.disp.createDescriptorSetLayout(&layoutCreateInfo, nullptr, &data.descriptorLayouts[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
   
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = data.descriptorPool;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT; 
    allocInfo.pSetLayouts = data.descriptorLayouts;
    if ( init.disp.allocateDescriptorSets(&allocInfo, data.descriptorSets) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
    return 0;
}

int VkRenderer::update_descriptor_sets() {
    VkDescriptorBufferInfo cameraBufferInfo = {};
    cameraBufferInfo.buffer = data.uniformBuffers[0].first;
    cameraBufferInfo.offset = 0;
    cameraBufferInfo.range = VK_WHOLE_SIZE;
    
    VkDescriptorBufferInfo modelBufferInfo = {};
    modelBufferInfo.buffer = data.uniformBuffers[1].first;
    modelBufferInfo.offset = 0;
    modelBufferInfo.range = VK_WHOLE_SIZE;
    
    std::vector<VkDescriptorImageInfo> textureInfos(data.textures.size());
    for(int i = 0; i < data.textures.size(); ++i) {
        textureInfos[i].sampler = data.textures[i].sampler;
        textureInfos[i].imageView = data.textures[i].view;
        textureInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    

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
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, data.descriptorSets[data.current_frame], 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &cameraBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, data.descriptorSets[data.current_frame], 1, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &modelBufferInfo, nullptr },
        //--- Textures ---//
            };
    if(data.textures.size() > 0) {
        writeDescriptorSets.push_back({ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, 
            nullptr, data.descriptorSets[data.current_frame],
            2, 0, (u32)textureInfos.size(),
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            textureInfos.data(), nullptr, nullptr 
        });
    }

    init.disp.updateDescriptorSets(writeDescriptorSets.size(),writeDescriptorSets.data(), 0,nullptr);
    return 0;
}
