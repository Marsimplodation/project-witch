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

    VkAttachmentDescription color_attachment = {};
    color_attachment.format = init.swapchain.image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {color_attachment, depthAttachment};
    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = attachments.size();
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (init.disp.createRenderPass(&render_pass_info, nullptr, &data.render_pass) != VK_SUCCESS) {
        std::cout << "failed to create render pass\n";
        return -1; // failed to create render pass!
    }
    color_attachment.format = VK_FORMAT_R8G8B8A8_SRGB;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachments = {color_attachment, depthAttachment};
    if (init.disp.createRenderPass(&render_pass_info, nullptr, &data.offscreen_pass) != VK_SUCCESS) {
        std::cout << "failed to create render pass\n";
        return -1; // failed to create render pass!
    }
    
    render_pass_info.attachmentCount = 1;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachments = {depthAttachment};
    depthAttachmentRef.attachment = 0;
    subpass.colorAttachmentCount = 0;
    subpass.pColorAttachments = VK_NULL_HANDLE;
    render_pass_info.pAttachments = attachments.data();
    if (init.disp.createRenderPass(&render_pass_info, nullptr, &data.shadow_pass) != VK_SUCCESS) {
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
    data.depthImages.resize(data.swapchain_image_views.size());
    data.depthImageMemorys.resize(data.swapchain_image_views.size());
    data.depthImageViews.resize(data.swapchain_image_views.size());
    data.shadow_images.resize(data.swapchain_image_views.size());
    data.shadow_image_views.resize(data.swapchain_image_views.size());
    data.shadow_image_memory.resize(data.swapchain_image_views.size());
    data.shadow_framebuffers.resize(data.swapchain_image_views.size());

    for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
        createImage(data.depthImages[i],
                    data.depthImageViews[i],
                    data.depthImageMemorys[i],
                    depthFormat,
                    extent,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_IMAGE_ASPECT_DEPTH_BIT);
        extent.width = SHADOW_MAP_RES;
        extent.height = SHADOW_MAP_RES; 
    }

    for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
        data.shadow_images[i].resize(SHADOW_CASCADES);
        data.shadow_image_memory[i].resize(SHADOW_CASCADES);
        data.shadow_image_views[i].resize(SHADOW_CASCADES);
        data.shadow_framebuffers[i].resize(SHADOW_CASCADES);
        for (int j = 0; j < SHADOW_CASCADES; ++j){
            createImage(data.shadow_images[i][j],
                        data.shadow_image_views[i][j],
                        data.shadow_image_memory[i][j],
                        depthFormat,
                        extent,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        VK_IMAGE_ASPECT_DEPTH_BIT);
        }
    }
}


int VkRenderer::create_framebuffers() {
    data.swapchain_images = init.swapchain.get_images().value();
    data.swapchain_image_views = init.swapchain.get_image_views().value();

    data.framebuffers.resize(data.swapchain_image_views.size());
    data.offscreen_framebuffers.resize(data.swapchain_image_views.size());
    createDepthResources();
    for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
        VkImageView attachments[] = { data.swapchain_image_views[i], data.depthImageViews[i]};

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = data.render_pass;
        framebuffer_info.attachmentCount = 2;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = init.swapchain.extent.width;
        framebuffer_info.height = init.swapchain.extent.height;
        framebuffer_info.layers = 1;

        if (init.disp.createFramebuffer(&framebuffer_info, nullptr, &data.framebuffers[i]) != VK_SUCCESS) {
            return -1; // failed to create framebuffer
        }
    }
    
    data.offscreen_images.resize(data.swapchain_image_views.size());
    data.offscreen_image_memory.resize(data.swapchain_image_views.size());
    data.offscreen_image_views.resize(data.swapchain_image_views.size());

    for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
        VkExtent2D extent;
        extent.width = std::min((u32)data.gui.previewSize.x, init.swapchain.extent.width); 
        extent.height = std::min((u32)data.gui.previewSize.y, init.swapchain.extent.height); 

        createImage(data.offscreen_images[i],
                    data.offscreen_image_views[i],
                    data.offscreen_image_memory[i],
                    VK_FORMAT_R8G8B8A8_SRGB,
                    extent,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT
                    );
    }
    for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
        std::array<VkImageView, 2> attachments = { data.offscreen_image_views[i], data.depthImageViews[i]};

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = data.offscreen_pass;
        framebuffer_info.attachmentCount = 2;
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = std::min((u32)data.gui.previewSize.x, init.swapchain.extent.width); 
        framebuffer_info.height = std::min((u32)data.gui.previewSize.y, init.swapchain.extent.height); 
        framebuffer_info.layers = 1;
        if (init.disp.createFramebuffer(&framebuffer_info, nullptr, &data.offscreen_framebuffers[i]) != VK_SUCCESS) {
            return -1; // failed to create framebuffer
        }
    }
    VkExtent2D extent;
    extent.width = SHADOW_MAP_RES;
    extent.height = SHADOW_MAP_RES; 
    for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
        for (size_t j = 0; j < SHADOW_CASCADES; j++) {
            std::array<VkImageView, 1> attachments = { data.shadow_image_views[i][j]};
            VkFramebufferCreateInfo framebuffer_info = {};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = data.shadow_pass;
            framebuffer_info.attachmentCount = 1;
            framebuffer_info.pAttachments = attachments.data();
            framebuffer_info.width = extent.width; 
            framebuffer_info.height = extent.height;
            framebuffer_info.layers = 1;
            if (init.disp.createFramebuffer(&framebuffer_info, nullptr, &data.shadow_framebuffers[i][j]) != VK_SUCCESS) {
                return -1; // failed to create framebuffer
            }
        }
    }
    return 0;
}


int VkRenderer::recreate_swapchain() {
    data.swapchain_out_of_date = false;
    init.disp.deviceWaitIdle();

    init.disp.destroyCommandPool(data.command_pool, nullptr);

    for (auto framebuffer : data.framebuffers) {
        init.disp.destroyFramebuffer(framebuffer, nullptr);
    }
    for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
        init.disp.destroyFramebuffer(data.offscreen_framebuffers[i], nullptr);
        init.disp.destroyImage(data.offscreen_images[i], nullptr);
        init.disp.destroyImageView(data.offscreen_image_views[i], nullptr);
        init.disp.freeMemory(data.offscreen_image_memory[i], nullptr);
        init.disp.destroyImage(data.depthImages[i], nullptr);
        init.disp.destroyImageView(data.depthImageViews[i], nullptr);
        init.disp.freeMemory(data.depthImageMemorys[i], nullptr);
    }

    for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
        for (size_t j = 0; j < SHADOW_CASCADES; j++) {
            init.disp.destroyFramebuffer(data.shadow_framebuffers[i][j], nullptr);
            init.disp.destroyImage(data.shadow_images[i][j], nullptr);
            init.disp.destroyImageView(data.shadow_image_views[i][j], nullptr);
            init.disp.freeMemory(data.shadow_image_memory[i][j], nullptr);
        }
    }

    init.swapchain.destroy_image_views(data.swapchain_image_views);

    if (0 != create_swapchain()) return -1;
    if (0 != create_framebuffers()) return -1;
    if (0 != create_command_pool()) return -1;
    if (0 != create_command_buffers()) return -1;
    return 0;
}
