#include "VkRenderer.h"
#include <iostream>
#include <vulkan/vulkan_core.h>


int VkRenderer::createRenderPass() {
            
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findSupportedFormat(init.physicalDevice,
    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
    VK_IMAGE_TILING_OPTIMAL,
    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = init.swapchain.image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (init.disp.createRenderPass(&renderPassInfo, nullptr, &data.renderPass) != VK_SUCCESS) {
        std::cout << "failed to create render pass\n";
        return -1; // failed to create render pass!
    }
    colorAttachment.format = VK_FORMAT_R8G8B8A8_SRGB;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachments = {colorAttachment, depthAttachment};
    if (init.disp.createRenderPass(&renderPassInfo, nullptr, &data.offscreenPass) != VK_SUCCESS) {
        std::cout << "failed to create render pass\n";
        return -1; // failed to create render pass!
    }
    
    renderPassInfo.attachmentCount = 1;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachments = {depthAttachment};
    depthAttachmentRef.attachment = 0;
    subpass.colorAttachmentCount = 0;
    subpass.pColorAttachments = VK_NULL_HANDLE;
    renderPassInfo.pAttachments = attachments.data();
    if (init.disp.createRenderPass(&renderPassInfo, nullptr, &data.shadowPass) != VK_SUCCESS) {
        std::cout << "failed to create render pass\n";
        return -1; // failed to create render pass!
    }
    createImageSampler();

    return 0;
}

void VkRenderer::createDepthResources() {
    VkFormat depthFormat = findSupportedFormat(init.physicalDevice,
    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
    VK_IMAGE_TILING_OPTIMAL,
    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
    VkExtent2D extent;
    extent.width = init.swapchain.extent.width;
    extent.height = init.swapchain.extent.height;
    data.depthImages.resize(data.swapchainImageViews.size());
    data.depthImageMemorys.resize(data.swapchainImageViews.size());
    data.depthImageViews.resize(data.swapchainImageViews.size());
    data.shadowImages.resize(data.swapchainImageViews.size());
    data.shadowImageViews.resize(data.swapchainImageViews.size());
    data.shadowImageMemory.resize(data.swapchainImageViews.size());
    data.shadowFramebuffers.resize(data.swapchainImageViews.size());

    for (size_t i = 0; i < data.swapchainImageViews.size(); i++) {
        createImage(data.depthImages[i],
                    data.depthImageViews[i],
                    data.depthImageMemorys[i],
                    depthFormat,
                    extent,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    for (size_t i = 0; i < data.swapchainImageViews.size(); i++) {
        data.shadowImages[i].resize(SHADOW_CASCADES);
        data.shadowImageMemory[i].resize(SHADOW_CASCADES);
        data.shadowImageViews[i].resize(SHADOW_CASCADES);
        data.shadowFramebuffers[i].resize(SHADOW_CASCADES);
        for (int j = 0; j < SHADOW_CASCADES; ++j){
            extent.width = SHADOW_MAP_RES[j];
            extent.height = SHADOW_MAP_RES[j]; 
            createImage(data.shadowImages[i][j],
                        data.shadowImageViews[i][j],
                        data.shadowImageMemory[i][j],
                        depthFormat,
                        extent,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        VK_IMAGE_ASPECT_DEPTH_BIT);
        }
    }
}


int VkRenderer::createFramebuffers() {
    data.swapchainImages = init.swapchain.get_images().value();
    data.swapchainImageViews = init.swapchain.get_image_views().value();

    data.framebuffers.resize(data.swapchainImageViews.size());
    data.offscreenFramebuffers.resize(data.swapchainImageViews.size());
    createDepthResources();
    for (size_t i = 0; i < data.swapchainImageViews.size(); i++) {
        VkImageView attachments[] = { data.swapchainImageViews[i], data.depthImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = data.renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = init.swapchain.extent.width;
        framebufferInfo.height = init.swapchain.extent.height;
        framebufferInfo.layers = 1;

        if (init.disp.createFramebuffer(&framebufferInfo, nullptr, &data.framebuffers[i]) != VK_SUCCESS) {
            return -1; // failed to create framebuffer
        }
    }
    
    data.offscreenImages.resize(data.swapchainImageViews.size());
    data.offscreenImageMemory.resize(data.swapchainImageViews.size());
    data.offscreenImageViews.resize(data.swapchainImageViews.size());

    for (size_t i = 0; i < data.swapchainImageViews.size(); i++) {
        VkExtent2D extent;
        extent.width = std::min((u32)data.gui.previewSize.x, init.swapchain.extent.width); 
        extent.height = std::min((u32)data.gui.previewSize.y, init.swapchain.extent.height); 

        createImage(data.offscreenImages[i],
                    data.offscreenImageViews[i],
                    data.offscreenImageMemory[i],
                    VK_FORMAT_R8G8B8A8_SRGB,
                    extent,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT
                    );
    }
    for (size_t i = 0; i < data.swapchainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = { data.offscreenImageViews[i], data.depthImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = data.offscreenPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = std::min((u32)data.gui.previewSize.x, init.swapchain.extent.width); 
        framebufferInfo.height = std::min((u32)data.gui.previewSize.y, init.swapchain.extent.height); 
        framebufferInfo.layers = 1;
        if (init.disp.createFramebuffer(&framebufferInfo, nullptr, &data.offscreenFramebuffers[i]) != VK_SUCCESS) {
            return -1; // failed to create framebuffer
        }
    }
    VkExtent2D extent;
    for (size_t i = 0; i < data.swapchainImageViews.size(); i++) {
        for (size_t j = 0; j < SHADOW_CASCADES; j++) {
            extent.width = SHADOW_MAP_RES[j];
            extent.height = SHADOW_MAP_RES[j]; 
            std::array<VkImageView, 1> attachments = { data.shadowImageViews[i][j]};
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = data.shadowPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = extent.width; 
            framebufferInfo.height = extent.height;
            framebufferInfo.layers = 1;
            if (init.disp.createFramebuffer(&framebufferInfo, nullptr, &data.shadowFramebuffers[i][j]) != VK_SUCCESS) {
                return -1; // failed to create framebuffer
            }
        }
    }
    return 0;
}


int VkRenderer::recreateSwapchain() {
    data.swapchainOutOfDate = false;
    init.disp.deviceWaitIdle();

    init.disp.destroyCommandPool(data.commandPool, nullptr);

    for (auto framebuffer : data.framebuffers) {
        init.disp.destroyFramebuffer(framebuffer, nullptr);
    }
    for (size_t i = 0; i < data.swapchainImageViews.size(); i++) {
        init.disp.destroyFramebuffer(data.offscreenFramebuffers[i], nullptr);
        init.disp.destroyImage(data.offscreenImages[i], nullptr);
        init.disp.destroyImageView(data.offscreenImageViews[i], nullptr);
        init.disp.freeMemory(data.offscreenImageMemory[i], nullptr);
        init.disp.destroyImage(data.depthImages[i], nullptr);
        init.disp.destroyImageView(data.depthImageViews[i], nullptr);
        init.disp.freeMemory(data.depthImageMemorys[i], nullptr);
    }

    for (size_t i = 0; i < data.swapchainImageViews.size(); i++) {
        for (size_t j = 0; j < SHADOW_CASCADES; j++) {
            init.disp.destroyFramebuffer(data.shadowFramebuffers[i][j], nullptr);
            init.disp.destroyImage(data.shadowImages[i][j], nullptr);
            init.disp.destroyImageView(data.shadowImageViews[i][j], nullptr);
            init.disp.freeMemory(data.shadowImageMemory[i][j], nullptr);
        }
    }

    init.swapchain.destroy_image_views(data.swapchainImageViews);

    if (0 != createSwapchain()) return -1;
    if (0 != createFramebuffers()) return -1;
    if (0 != createCommandPool()) return -1;
    if (0 != createCommandBuffers()) return -1;
    return 0;
}
