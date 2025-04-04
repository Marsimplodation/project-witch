#include "VkRenderer.h"
#include "types/defines.h"
#include "types/types.h"
#include <cstdio>
#include <vulkan/vulkan_core.h>
int VkRenderer::createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[2] = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = (5 + MAX_MATERIALS) * MAX_FRAMES_IN_FLIGHT; // Change to 3 for the 3 descriptors (raygen, miss, hit)
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = (SHADOW_CASCADES + MAX_TEXTURES) * MAX_FRAMES_IN_FLIGHT; // Change to 3 for the 3 descriptors (raygen, miss, hit)
    
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
    poolSizes[0].descriptorCount = (4 + MAX_MATERIALS) * SHADOW_CASCADES * MAX_FRAMES_IN_FLIGHT; // Change to 3 for the 3 descriptors (raygen, miss, hit)
    poolSizes[1].descriptorCount = (MAX_TEXTURES) * SHADOW_CASCADES * MAX_FRAMES_IN_FLIGHT; // Change to 3 for the 3 descriptors (raygen, miss, hit)
    poolInfo.maxSets = SHADOW_CASCADES * MAX_FRAMES_IN_FLIGHT;  // Only need 1 descriptor set (if you're allocating 1 per frame)
    result = init.disp.createDescriptorPool(&poolInfo, nullptr, &data.descriptorShadowPool); 
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool");
    }

    return 0;
}

int VkRenderer::createDescriptorLayout() {
    VkDescriptorSetLayoutBinding cameraBinding = {};
    cameraBinding.binding = 0;
    cameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    cameraBinding.descriptorCount = 1;
    cameraBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  // This buffer is used by the closest hit shader.
    
    VkDescriptorSetLayoutBinding modelBinding = {};
    modelBinding.binding = 1;
    modelBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    modelBinding.descriptorCount = 1;
    modelBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  // This buffer is used by the closest hit shader.

    VkDescriptorSetLayoutBinding textureLayoutBinding{};
    textureLayoutBinding.binding = 2;  // Binding index
    textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureLayoutBinding.descriptorCount = SHADOW_CASCADES + MAX_TEXTURES;
    textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    textureLayoutBinding.pImmutableSamplers = nullptr;  // No static samplers
    
    VkDescriptorSetLayoutBinding lightBinding = {};
    lightBinding.binding = 3;
    lightBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    lightBinding.descriptorCount = 1;
    lightBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;  // This buffer is used by the closest hit shader.
    
    VkDescriptorSetLayoutBinding materialBinding = {};
    materialBinding.binding = 4;
    materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    materialBinding.descriptorCount = 1;
    materialBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;  // This buffer is used by the closest hit shader.
    
    VkDescriptorSetLayoutBinding pointLightBinding = {};
    pointLightBinding.binding = 5;
    pointLightBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pointLightBinding.descriptorCount = 1;
    pointLightBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;  // This buffer is used by the closest hit shader.

    

    std::vector<VkDescriptorSetLayoutBinding> bindings = {cameraBinding, modelBinding, textureLayoutBinding, lightBinding, materialBinding, pointLightBinding};

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = bindings.size();  // Number of bindings
    layoutCreateInfo.pBindings = bindings.data();
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

    //------- SHADOWS -----//
    textureLayoutBinding.descriptorCount = MAX_TEXTURES;
    bindings = {modelBinding, textureLayoutBinding, lightBinding, materialBinding};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = bindings.size();  // Number of bindings
    layoutCreateInfo.pBindings = bindings.data();
    for(int c = 0; c < SHADOW_CASCADES; ++c) {
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i){
            VkResult result = init.disp.createDescriptorSetLayout(&layoutCreateInfo, nullptr, &data.descriptorShadowLayouts[c][i]);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }
        }
       
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = data.descriptorShadowPool;
        allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT; 
        allocInfo.pSetLayouts = data.descriptorShadowLayouts[c];
        if ( init.disp.allocateDescriptorSets(&allocInfo, data.descriptorShadowSets[c]) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
    }
    return 0;
}

#define CAMERA_BUFFER 0
#define MODEL_BUFFER 1
#define LIGHT_BUFFER 2 + SHADOW_CASCADES
#define MATERIAL_BUFFER 3 + SHADOW_CASCADES
#define POINT_LIGHT_BUFFER 4 + SHADOW_CASCADES

int VkRenderer::updateDescriptorSets() {
    VkDescriptorBufferInfo cameraBufferInfo = {};
    cameraBufferInfo.buffer = data.uniformBuffers[data.currentFrame][CAMERA_BUFFER].first;
    cameraBufferInfo.offset = 0;
    cameraBufferInfo.range = VK_WHOLE_SIZE;
    
    VkDescriptorBufferInfo modelBufferInfo = {};
    modelBufferInfo.buffer = data.uniformBuffers[data.currentFrame][MODEL_BUFFER].first;
    modelBufferInfo.offset = 0;
    modelBufferInfo.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo lightBufferInfo = {};
    lightBufferInfo.buffer = data.uniformBuffers[data.currentFrame][LIGHT_BUFFER].first;
    lightBufferInfo.offset = 0;
    lightBufferInfo.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo materialBufferInfo = {};
    materialBufferInfo.buffer = data.uniformBuffers[data.currentFrame][MATERIAL_BUFFER].first;
    materialBufferInfo.offset = 0;
    materialBufferInfo.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo pointLightBufferInfo = {};
    pointLightBufferInfo.buffer = data.uniformBuffers[data.currentFrame][POINT_LIGHT_BUFFER].first;
    pointLightBufferInfo.offset = 0;
    pointLightBufferInfo.range = VK_WHOLE_SIZE;

    
    std::vector<VkDescriptorImageInfo> textureInfos(MAX_TEXTURES + SHADOW_CASCADES);
    // Initialize all slots to VK_NULL_HANDLE (optional, but safe)
    for (int i = 0; i < MAX_TEXTURES + SHADOW_CASCADES; ++i) {
        textureInfos[i].sampler = data.textures[0].sampler;
        textureInfos[i].imageView = data.textures[0].view;
        textureInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    for(int i = 0; i < SHADOW_CASCADES; ++i) {
        textureInfos[i].sampler = data.imageSampler;
        textureInfos[i].imageView = data.shadowImageViews[data.currentImgIndex][i];
        textureInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    for(int i = 0; i < data.textures.size(); ++i) {
        textureInfos[i + SHADOW_CASCADES].sampler = data.textures[i].sampler;
        textureInfos[i + SHADOW_CASCADES].imageView = data.textures[i].view;
        textureInfos[i + SHADOW_CASCADES].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, data.descriptorSets[data.currentFrame], 0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &cameraBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, data.descriptorSets[data.currentFrame], 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &modelBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, data.descriptorSets[data.currentFrame], 3, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &lightBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, data.descriptorSets[data.currentFrame], 4, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &materialBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, data.descriptorSets[data.currentFrame], 5, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &pointLightBufferInfo, nullptr },
        //--- Textures ---//
            };
    if(textureInfos.size() > 0) {
        writeDescriptorSets.push_back({ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, 
            nullptr, data.descriptorSets[data.currentFrame],
            2, 0, (u32)textureInfos.size(),
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            textureInfos.data(), nullptr, nullptr 
        });
    }

    init.disp.updateDescriptorSets(writeDescriptorSets.size(),writeDescriptorSets.data(), 0,nullptr);
    return 0;
}

int VkRenderer::updateShadowDescriptorSets() {

    VkDescriptorBufferInfo lightBufferInfo = {};
    lightBufferInfo.buffer = data.uniformBuffers[data.currentFrame][LIGHT_BUFFER].first;
    lightBufferInfo.offset = 0;
    lightBufferInfo.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo materialBufferInfo = {};
    materialBufferInfo.buffer = data.uniformBuffers[data.currentFrame][MATERIAL_BUFFER].first;
    materialBufferInfo.offset = 0;
    materialBufferInfo.range = VK_WHOLE_SIZE;
    
    std::vector<VkDescriptorImageInfo> textureInfos(MAX_TEXTURES);
    // Initialize all slots to VK_NULL_HANDLE (optional, but safe)
    for (int i = 0; i < MAX_TEXTURES; ++i) {
        textureInfos[i].sampler = data.textures[0].sampler;
        textureInfos[i].imageView = data.textures[0].view;
        textureInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    for(int i = 0; i < data.textures.size(); ++i) {
        textureInfos[i].sampler = data.textures[i].sampler;
        textureInfos[i].imageView = data.textures[i].view;
        textureInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    
    for(int c = 0; c < SHADOW_CASCADES; ++c) {
        VkDescriptorBufferInfo modelBufferInfo = {};
        modelBufferInfo.buffer = data.uniformBuffers[data.currentFrame][MODEL_BUFFER + c + 1].first;
        modelBufferInfo.offset = 0;
        modelBufferInfo.range = VK_WHOLE_SIZE;

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
            { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, data.descriptorShadowSets[c][data.currentFrame], 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &modelBufferInfo, nullptr },
            { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, data.descriptorShadowSets[c][data.currentFrame], 3, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &lightBufferInfo, nullptr },
            { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, data.descriptorShadowSets[c][data.currentFrame], 4, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &materialBufferInfo, nullptr },
            //--- Textures ---//
                };
        if(textureInfos.size() > 0) {
            writeDescriptorSets.push_back({ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, 
                nullptr, data.descriptorShadowSets[c][data.currentFrame],
                2, 0, (u32)textureInfos.size(),
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                textureInfos.data(), nullptr, nullptr 
            });
        }

        init.disp.updateDescriptorSets(writeDescriptorSets.size(),writeDescriptorSets.data(), 0,nullptr);
    }
    return 0;
}
