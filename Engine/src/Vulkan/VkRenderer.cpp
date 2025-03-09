#include "VkRenderer.h"
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <vulkan/vulkan_core.h>

GLFWwindow* VkRenderer::createWindowGlfw(const char* windowName, bool resize) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (!resize) glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return glfwCreateWindow(1024, 1024, windowName, NULL, NULL);
}

void VkRenderer::destroyWindowGlfw(GLFWwindow* window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

VkSurfaceKHR VkRenderer::createSurfaceGlfw(VkInstance instance, GLFWwindow* window, VkAllocationCallbacks* allocator) {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult err = glfwCreateWindowSurface(instance, window, allocator, &surface);
    if (err) {
        const char* errorMsg;
        int ret = glfwGetError(&errorMsg);
        if (ret != 0) {
            std::cout << ret << " ";
            if (errorMsg != nullptr) std::cout << errorMsg;
            std::cout << "\n";
        }
        surface = VK_NULL_HANDLE;
    }
    return surface;
}

int VkRenderer::deviceInitialization() {
    init.window = createWindowGlfw("Vulkan Triangle", true);

    vkb::InstanceBuilder instanceBuilder;
    instanceBuilder.set_app_version(VK_API_VERSION_1_3);
    auto instanceRet = instanceBuilder.use_default_debug_messenger().request_validation_layers().build();
    if (!instanceRet) {
        std::cout << instanceRet.error().message() << "\n";
        return -1;
    }
    init.instance = instanceRet.value();

    init.instDisp = init.instance.make_table();

    init.surface = createSurfaceGlfw(init.instance, init.window);

    vkb::PhysicalDeviceSelector physDeviceSelector(init.instance);
    VkPhysicalDeviceVulkan12Features vk12Features{};
    vk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vk12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    vk12Features.descriptorIndexing = VK_TRUE;
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.fillModeNonSolid = VK_TRUE;
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.multiDrawIndirect = VK_TRUE;
    physDeviceSelector.set_required_features(deviceFeatures);
    physDeviceSelector.set_required_features_12(vk12Features);
    physDeviceSelector.add_required_extension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    physDeviceSelector.add_required_extension(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
    auto physDeviceRet = physDeviceSelector.set_surface(init.surface).select();
    if (!physDeviceRet) {
        std::cout << physDeviceRet.error().message() << "\n";
        return -1;
    }
    vkb::PhysicalDevice physicalDevice = physDeviceRet.value();
    init.physicalDevice = physicalDevice;

    vkb::DeviceBuilder deviceBuilder{ physicalDevice };
    auto deviceRet = deviceBuilder.build();
    if (!deviceRet) {
        std::cout << deviceRet.error().message() << "\n";
        return -1;
    }
    init.device = deviceRet.value();

    init.disp = init.device.make_table();

    return 0;
}

int VkRenderer::createSwapchain() {
    vkb::SwapchainBuilder swapchainBuilder{ init.device };
    VkSurfaceFormatKHR format{
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    };
    swapchainBuilder.set_desired_format(format);
    auto swapRet = swapchainBuilder.set_old_swapchain(init.swapchain).build();
    if (!swapRet) {
        std::cout << swapRet.error().message() << " " << swapRet.vk_result() << "\n";
        return -1;
    }
    vkb::destroy_swapchain(init.swapchain);
    init.swapchain = swapRet.value();
    return 0;
}

int VkRenderer::getQueues() {
    auto gq = init.device.get_queue(vkb::QueueType::graphics);
    if (!gq.has_value()) {
        std::cout << "failed to get graphics queue: " << gq.error().message() << "\n";
        return -1;
    }
    data.graphicsQueue = gq.value();

    auto pq = init.device.get_queue(vkb::QueueType::present);
    if (!pq.has_value()) {
        std::cout << "failed to get present queue: " << pq.error().message() << "\n";
        return -1;
    }
    data.presentQueue = pq.value();
    return 0;
}



int VkRenderer::drawFrame() {
    init.disp.waitForFences(1, &data.inFlightFences[data.currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    if (data.swapchainOutOfDate) {
        return recreateSwapchain();
    }
    VkResult result = init.disp.acquireNextImageKHR(
        init.swapchain, UINT64_MAX, data.availableSemaphores[data.currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || data.swapchainOutOfDate) {
        return recreateSwapchain();
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cout << "failed to acquire swapchain image. Error " << result << "\n";
        return -1;
    }

    data.currentImgIndex = imageIndex;
    if (data.imageInFlight[imageIndex] != VK_NULL_HANDLE) {
        init.disp.waitForFences(1, &data.imageInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    data.imageInFlight[imageIndex] = data.inFlightFences[data.currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { data.availableSemaphores[data.currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    recordCommandBuffer(imageIndex);
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &data.commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = { data.finishedSemaphore[data.currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    init.disp.resetFences(1, &data.inFlightFences[data.currentFrame]);

    if (init.disp.queueSubmit(data.graphicsQueue, 1, &submitInfo, data.inFlightFences[data.currentFrame]) != VK_SUCCESS) {
        std::cout << "failed to submit draw command buffer\n";
        return -1; //"failed to submit draw command buffer
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { init.swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = init.disp.queuePresentKHR(data.presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return recreateSwapchain();
    } else if (result != VK_SUCCESS) {
        std::cout << "failed to present swapchain image\n";
        return -1;
    }

    data.currentFrame = (data.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    return 0;
}

void VkRenderer::cleanup() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        init.disp.destroySemaphore(data.finishedSemaphore[i], nullptr);
        init.disp.destroySemaphore(data.availableSemaphores[i], nullptr);
        init.disp.destroyFence(data.inFlightFences[i], nullptr);
        init.disp.destroyDescriptorSetLayout(data.descriptorLayouts[i], nullptr);
        init.disp.destroyDescriptorSetLayout(data.descriptorShadowLayouts[i], nullptr);
    }
    init.disp.destroyDescriptorPool(data.descriptorPool, nullptr);
    init.disp.destroyDescriptorPool(data.descriptorShadowPool, nullptr);

    init.disp.destroyCommandPool(data.commandPool, nullptr);
    destroyBuffers();

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
    for (size_t i = 0; i < data.textures.size(); i++) {
        init.disp.destroyImage(data.textures[i].image, nullptr);
        init.disp.destroyImageView(data.textures[i].view, nullptr);
        init.disp.freeMemory(data.textures[i].memory, nullptr);
        init.disp.destroySampler(data.textures[i].sampler, nullptr);
    }
    for (size_t i = 0; i < data.swapchainImageViews.size(); i++) {
        for (size_t j = 0; j < SHADOW_CASCADES; j++) {
            init.disp.destroyFramebuffer(data.shadowFramebuffers[i][j], nullptr);
            init.disp.destroyImage(data.shadowImages[i][j], nullptr);
            init.disp.destroyImageView(data.shadowImageViews[i][j], nullptr);
            init.disp.freeMemory(data.shadowImageMemory[i][j], nullptr);
        }
    }


    init.disp.destroyPipeline(data.graphicsPipeline, nullptr);
    init.disp.destroyPipeline(data.shadowPipeline, nullptr);
    init.disp.destroyPipeline(data.offscreenPipeline, nullptr);
    init.disp.destroyPipelineLayout(data.pipelineLayout, nullptr);
    init.disp.destroyPipelineLayout(data.shadowPipelineLayout, nullptr);
    init.disp.destroyRenderPass(data.shadowPass, nullptr);
    init.disp.destroyRenderPass(data.renderPass, nullptr);
    init.disp.destroyRenderPass(data.offscreenPass, nullptr);
    init.disp.destroySampler(data.imageSampler, nullptr);

    init.swapchain.destroy_image_views(data.swapchainImageViews);

    vkb::destroy_swapchain(init.swapchain);
    vkb::destroy_device(init.device);
    vkb::destroy_surface(init.instance, init.surface);
    vkb::destroy_instance(init.instance);
    destroyWindowGlfw(init.window);
}

