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
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.fillModeNonSolid = VK_TRUE;
        phys_device_selector.set_required_features(deviceFeatures);
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
        init.disp.destroyImage(data.depthImage, nullptr);
        init.disp.destroyImageView(data.depthImageView, nullptr);
        init.disp.freeMemory(data.depthImageMemory, nullptr);

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
