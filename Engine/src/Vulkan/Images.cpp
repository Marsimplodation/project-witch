#include "VkRenderer.h"
#include <vulkan/vulkan_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../includes/stb_image.h"

    
VkFormat VkRenderer::findSupportedFormat(vkb::PhysicalDevice & physicalDevice,
                             const std::vector<VkFormat>& candidates,
                             VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

void VkRenderer::createImage(VkImage & image, VkImageView & view, VkDeviceMemory & memory,
                 VkFormat requestedFormat, VkExtent2D extent,
                VkImageUsageFlags flags, VkImageAspectFlags aspectFlags) {
        VkImageCreateInfo image_info = {};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.format = requestedFormat; 
        image_info.extent.width = extent.width;
        image_info.extent.height = extent.height;
        image_info.extent.depth = 1;
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.usage = flags; 
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (init.disp.createImage(&image_info, nullptr, &image) != VK_SUCCESS) {
            return; // failed to create offscreen image
        }

        // Allocate and bind memory for the image
        VkMemoryRequirements mem_requirements;
        init.disp.getImageMemoryRequirements(image, &mem_requirements);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (init.disp.allocateMemory(&alloc_info, nullptr, &memory) != VK_SUCCESS) {
            return; // failed to allocate offscreen image memory
        }

        init.disp.bindImageMemory(image, memory, 0);
        VkImageViewCreateInfo view_info = {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = requestedFormat; 
        view_info.subresourceRange.aspectMask = aspectFlags;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        if (init.disp.createImageView(&view_info, nullptr, &view) != VK_SUCCESS) {
            return; // failed to create offscreen image view
        }
}


void VkRenderer::TransitionImageLayout(
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

void VkRenderer::copyDataToImage(VkImage image,const std::vector<glm::vec4>& textureData, VkExtent2D extent, VkDeviceSize dataSize) {
    // Step 1: Create a staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(dataSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &stagingBuffer, &stagingBufferMemory);

    // Step 2: Map memory and copy data
    void* mappedData;
    vkMapMemory(init.device, stagingBufferMemory, 0, dataSize, 0, &mappedData);
    memcpy(mappedData, textureData.data(), dataSize);
    vkUnmapMemory(init.device, stagingBufferMemory);

    // Step 4: Copy buffer data to image
    //copy data to actual buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = data.command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    init.disp.allocateCommandBuffers(&allocInfo, &commandBuffer);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    init.disp.beginCommandBuffer(commandBuffer, &beginInfo);

    TransitionImageLayout(commandBuffer,
                      image,
                      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);


    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;  // Tightly packed
    region.bufferImageHeight = 0; // Tightly packed
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { extent.width, extent.height, 1 };
    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


    // Step 5: Transition image to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    TransitionImageLayout(commandBuffer,
                          image,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    init.disp.endCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    init.disp.queueSubmit(data.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    init.disp.queueWaitIdle(data.graphics_queue);
    init.disp.destroyBuffer(stagingBuffer, nullptr); 
    init.disp.freeMemory(stagingBufferMemory, nullptr); 
}


int VkRenderer::loadTexture(std::string path) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    std::vector<glm::vec4> textureData;
    if(!pixels) {
        printf("could not load: %s\n", path.c_str());
        return -1;
    }
    // Clear any existing data in the texture

    // Copy pixel data to the texture
    for (int y = 0; y < texHeight; ++y) {
        for (int x = 0; x < texWidth; ++x) {
            int pixelIndex = (y * texWidth + x) * 4; // RGBA channels
            float r = pixels[pixelIndex] / 255.0f;
            float g = pixels[pixelIndex + 1] / 255.0f;
            float b = pixels[pixelIndex + 2] / 255.0f;
            float a = pixels[pixelIndex + 3] / 255.0f;
            textureData.push_back({ r, g, b, a});
        }
    }
    
    // Free stb_image allocated memory
    stbi_image_free(pixels);
    printf("loaded texture %s\n", path.c_str());


    int index = data.textures.size();
    data.textures.push_back({});
    Texture & texture = data.textures[index];
    VkExtent2D extent = {(u32)texWidth, (u32)texHeight};
    createImage(texture.image, texture.view, texture.memory, VK_FORMAT_R32G32B32A32_SFLOAT, extent, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
    VkMemoryRequirements mem_requirements;
    init.disp.getImageMemoryRequirements(texture.image, &mem_requirements);
    VkDeviceSize dataSize = mem_requirements.size;

    
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;  // Linear filtering
    samplerInfo.minFilter = VK_FILTER_LINEAR;  // Linear filtering
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (init.disp.createSampler(&samplerInfo, nullptr, &texture.sampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create sampler!");
    }
    copyDataToImage(texture.image, textureData, extent, dataSize);


    return index;
}
