#include "VkRenderer.h"



    
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

void VkRenderer::createImage(Init& init, RenderData & data,
                 VkImage & image, VkImageView & view, VkDeviceMemory & memory,
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
        alloc_info.memoryTypeIndex = findMemoryType(init, mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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
