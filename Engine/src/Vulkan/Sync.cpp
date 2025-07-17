#include "VkRenderer.h"
#include "types/defines.h"
#include <iostream>
int VkRenderer::createSyncObjects() {
    data.availableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    data.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    data.finishedSemaphore.resize(init.swapchain.image_count);
    data.imageInFlight.resize(init.swapchain.image_count, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < init.swapchain.image_count; i++) {
        if ( init.disp.createSemaphore(&semaphoreInfo, nullptr, &data.finishedSemaphore[i]) != VK_SUCCESS) {
            std::cout << "failed to create sync objects\n";
            return -1; // failed to create synchronization objects for a frame
        }
    }
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (init.disp.createSemaphore(&semaphoreInfo, nullptr, &data.availableSemaphores[i]) != VK_SUCCESS ||
            init.disp.createFence(&fenceInfo, nullptr, &data.inFlightFences[i]) != VK_SUCCESS) {
            std::cout << "failed to create sync objects\n";
            return -1; // failed to create synchronization objects for a frame
        }
    }
    return 0;
}
