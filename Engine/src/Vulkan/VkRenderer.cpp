#include "VkRenderer.h"
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <vulkan/vulkan_core.h>

namespace VkRenderer {
    GLFWwindow* create_window_glfw(const char* window_name, bool resize) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        if (!resize) glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        return glfwCreateWindow(1024, 1024, window_name, NULL, NULL);
    }

    void destroy_window_glfw(GLFWwindow* window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    VkSurfaceKHR create_surface_glfw(VkInstance instance, GLFWwindow* window, VkAllocationCallbacks* allocator) {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkResult err = glfwCreateWindowSurface(instance, window, allocator, &surface);
        if (err) {
            const char* error_msg;
            int ret = glfwGetError(&error_msg);
            if (ret != 0) {
                std::cout << ret << " ";
                if (error_msg != nullptr) std::cout << error_msg;
                std::cout << "\n";
            }
            surface = VK_NULL_HANDLE;
        }
        return surface;
    }

    int device_initialization(Init& init) {
        init.window = create_window_glfw("Vulkan Triangle", true);

        vkb::InstanceBuilder instance_builder;
        auto instance_ret = instance_builder.use_default_debug_messenger().request_validation_layers().build();
        if (!instance_ret) {
            std::cout << instance_ret.error().message() << "\n";
            return -1;
        }
        init.instance = instance_ret.value();

        init.inst_disp = init.instance.make_table();

        init.surface = create_surface_glfw(init.instance, init.window);

        vkb::PhysicalDeviceSelector phys_device_selector(init.instance);
        auto phys_device_ret = phys_device_selector.set_surface(init.surface).select();
        if (!phys_device_ret) {
            std::cout << phys_device_ret.error().message() << "\n";
            return -1;
        }
        vkb::PhysicalDevice physical_device = phys_device_ret.value();
        init.physicalDevice = physical_device;

        vkb::DeviceBuilder device_builder{ physical_device };
        auto device_ret = device_builder.build();
        if (!device_ret) {
            std::cout << device_ret.error().message() << "\n";
            return -1;
        }
        init.device = device_ret.value();

        init.disp = init.device.make_table();

        return 0;
    }

    int create_swapchain(Init& init) {
        vkb::SwapchainBuilder swapchain_builder{ init.device };
        auto swap_ret = swapchain_builder.set_old_swapchain(init.swapchain).build();
        if (!swap_ret) {
            std::cout << swap_ret.error().message() << " " << swap_ret.vk_result() << "\n";
            return -1;
        }
        vkb::destroy_swapchain(init.swapchain);
        init.swapchain = swap_ret.value();
        return 0;
    }

    int get_queues(Init& init, RenderData& data) {
        auto gq = init.device.get_queue(vkb::QueueType::graphics);
        if (!gq.has_value()) {
            std::cout << "failed to get graphics queue: " << gq.error().message() << "\n";
            return -1;
        }
        data.graphics_queue = gq.value();

        auto pq = init.device.get_queue(vkb::QueueType::present);
        if (!pq.has_value()) {
            std::cout << "failed to get present queue: " << pq.error().message() << "\n";
            return -1;
        }
        data.present_queue = pq.value();
        return 0;
    }

    int create_off_screen_render_pass(Init& init, RenderData& data) {
        VkAttachmentDescription color_attachment = {};
        color_attachment.format = VK_FORMAT_R8G8B8A8_SRGB;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkAttachmentReference color_attachment_ref = {};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;

        if (init.disp.createRenderPass(&render_pass_info, nullptr, &data.offscreen_pass) != VK_SUCCESS) {
            std::cout << "failed to create render pass\n";
            return -1; // failed to create render pass!
        }
         //create sampler for image
        
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



        if (init.disp.createSampler(&samplerInfo, nullptr, &data.imageSampler) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create sampler!");
        }
        return 0;
    }


    int create_render_pass(Init& init, RenderData& data) {
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

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;
        

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;

        if (init.disp.createRenderPass(&render_pass_info, nullptr, &data.render_pass) != VK_SUCCESS) {
            std::cout << "failed to create render pass\n";
            return -1; // failed to create render pass!
        }
        return 0;
    }




    int create_framebuffers(Init& init, RenderData& data) {
        data.swapchain_images = init.swapchain.get_images().value();
        data.swapchain_image_views = init.swapchain.get_image_views().value();

        data.framebuffers.resize(data.swapchain_image_views.size());
        data.offscreen_framebuffers.resize(data.swapchain_image_views.size());

        for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
            VkImageView attachments[] = { data.swapchain_image_views[i] };

            VkFramebufferCreateInfo framebuffer_info = {};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = data.render_pass;
            framebuffer_info.attachmentCount = 1;
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

        for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
            VkImageCreateInfo image_info = {};
            image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_info.imageType = VK_IMAGE_TYPE_2D;
            image_info.format = VK_FORMAT_R8G8B8A8_SRGB; // Or a format matching your use case
            image_info.extent.width = data.gui.previewSize.x;
            image_info.extent.height = data.gui.previewSize.y;
            image_info.extent.depth = 1;
            image_info.mipLevels = 1;
            image_info.arrayLayers = 1;
            image_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            if (init.disp.createImage(&image_info, nullptr, &data.offscreen_images[i]) != VK_SUCCESS) {
                return -1; // failed to create offscreen image
            }

            // Allocate and bind memory for the image
            VkMemoryRequirements mem_requirements;
            init.disp.getImageMemoryRequirements(data.offscreen_images[i], &mem_requirements);

            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = mem_requirements.size;
            alloc_info.memoryTypeIndex = findMemoryType(init, mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            if (init.disp.allocateMemory(&alloc_info, nullptr, &data.offscreen_image_memory[i]) != VK_SUCCESS) {
                return -1; // failed to allocate offscreen image memory
            }

            init.disp.bindImageMemory(data.offscreen_images[i], data.offscreen_image_memory[i], 0);
        }
        
        data.offscreen_image_views.resize(data.swapchain_image_views.size());

        for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
            VkImageViewCreateInfo view_info = {};
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image = data.offscreen_images[i];
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = VK_FORMAT_R8G8B8A8_SRGB; // Must match the image format
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;

            if (init.disp.createImageView(&view_info, nullptr, &data.offscreen_image_views[i]) != VK_SUCCESS) {
                return -1; // failed to create offscreen image view
            }
        }

        
        for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
            VkImageView attachments[] = { data.offscreen_image_views[i] };

            VkFramebufferCreateInfo framebuffer_info = {};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = data.offscreen_pass;
            framebuffer_info.attachmentCount = 1;
            framebuffer_info.pAttachments = attachments;
            framebuffer_info.width = data.gui.previewSize.x; 
            framebuffer_info.height = data.gui.previewSize.y; 
            framebuffer_info.layers = 1;

            if (init.disp.createFramebuffer(&framebuffer_info, nullptr, &data.offscreen_framebuffers[i]) != VK_SUCCESS) {
                return -1; // failed to create framebuffer
            }
        }
        return 0;
    }


    int recreate_swapchain(Init& init, RenderData& data) {
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
        }

        init.swapchain.destroy_image_views(data.swapchain_image_views);

        if (0 != create_swapchain(init)) return -1;
        if (0 != create_framebuffers(init, data)) return -1;
        if (0 != create_command_pool(init, data)) return -1;
        if (0 != create_command_buffers(init, data)) return -1;
        return 0;
    }


    int draw_frame(Init& init, RenderData& data) {
        init.disp.waitForFences(1, &data.in_flight_fences[data.current_frame], VK_TRUE, UINT64_MAX);

        uint32_t image_index = 0;
        if (data.swapchain_out_of_date) {
            return recreate_swapchain(init, data);
        }
        VkResult result = init.disp.acquireNextImageKHR(
            init.swapchain, UINT64_MAX, data.available_semaphores[data.current_frame], VK_NULL_HANDLE, &image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || data.swapchain_out_of_date) {
            return recreate_swapchain(init, data);
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            std::cout << "failed to acquire swapchain image. Error " << result << "\n";
            return -1;
        }

        data.current_img_index = image_index;
        if (data.image_in_flight[image_index] != VK_NULL_HANDLE) {
            init.disp.waitForFences(1, &data.image_in_flight[image_index], VK_TRUE, UINT64_MAX);
        }
        data.image_in_flight[image_index] = data.in_flight_fences[data.current_frame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore wait_semaphores[] = { data.available_semaphores[data.current_frame] };
        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = wait_semaphores;
        submitInfo.pWaitDstStageMask = wait_stages;

        record_command_buffer(init, data, image_index);
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &data.command_buffers[image_index];

        VkSemaphore signal_semaphores[] = { data.finished_semaphore[data.current_frame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signal_semaphores;

        init.disp.resetFences(1, &data.in_flight_fences[data.current_frame]);

        if (init.disp.queueSubmit(data.graphics_queue, 1, &submitInfo, data.in_flight_fences[data.current_frame]) != VK_SUCCESS) {
            std::cout << "failed to submit draw command buffer\n";
            return -1; //"failed to submit draw command buffer
        }

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;

        VkSwapchainKHR swapChains[] = { init.swapchain };
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapChains;

        present_info.pImageIndices = &image_index;

        result = init.disp.queuePresentKHR(data.present_queue, &present_info);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            return recreate_swapchain(init, data);
        } else if (result != VK_SUCCESS) {
            std::cout << "failed to present swapchain image\n";
            return -1;
        }

        data.current_frame = (data.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
        return 0;
    }

    void cleanup(Init& init, RenderData& data) {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            init.disp.destroySemaphore(data.finished_semaphore[i], nullptr);
            init.disp.destroySemaphore(data.available_semaphores[i], nullptr);
            init.disp.destroyFence(data.in_flight_fences[i], nullptr);
            init.disp.destroyDescriptorSetLayout(data.descriptorLayouts[i], nullptr);
        }
        init.disp.destroyDescriptorPool(data.descriptorPool, nullptr);

        init.disp.destroyCommandPool(data.command_pool, nullptr);
        destroyBuffers(init, data);

        for (auto framebuffer : data.framebuffers) {
            init.disp.destroyFramebuffer(framebuffer, nullptr);
        }
        for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
            init.disp.destroyFramebuffer(data.offscreen_framebuffers[i], nullptr);
            init.disp.destroyImage(data.offscreen_images[i], nullptr);
            init.disp.destroyImageView(data.offscreen_image_views[i], nullptr);
            init.disp.freeMemory(data.offscreen_image_memory[i], nullptr);
        }

        init.disp.destroyPipeline(data.graphics_pipeline, nullptr);
        init.disp.destroyPipeline(data.offscreen_pipeline, nullptr);
        init.disp.destroyPipelineLayout(data.pipeline_layout, nullptr);
        init.disp.destroyRenderPass(data.render_pass, nullptr);
        init.disp.destroyRenderPass(data.offscreen_pass, nullptr);
        init.disp.destroySampler(data.imageSampler, nullptr);

        init.swapchain.destroy_image_views(data.swapchain_image_views);

        vkb::destroy_swapchain(init.swapchain);
        vkb::destroy_device(init.device);
        vkb::destroy_surface(init.instance, init.surface);
        vkb::destroy_instance(init.instance);
        destroy_window_glfw(init.window);
    }


}
