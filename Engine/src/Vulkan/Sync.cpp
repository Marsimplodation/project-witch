#include "VkRenderer.h"
#include <iostream>
namespace VkRenderer {
int create_sync_objects(Init& init, RenderData& data) {
        data.available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        data.finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
        data.in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
        data.image_in_flight.resize(init.swapchain.image_count, VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (init.disp.createSemaphore(&semaphore_info, nullptr, &data.available_semaphores[i]) != VK_SUCCESS ||
                init.disp.createSemaphore(&semaphore_info, nullptr, &data.finished_semaphore[i]) != VK_SUCCESS ||
                init.disp.createFence(&fence_info, nullptr, &data.in_flight_fences[i]) != VK_SUCCESS) {
                std::cout << "failed to create sync objects\n";
                return -1; // failed to create synchronization objects for a frame
            }
        }
        return 0;
    }
}
