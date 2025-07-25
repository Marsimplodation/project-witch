#ifndef EDITOR_MODULE_H
#define EDITOR_MODULE_H 
#include "imgui.h"
#include "types/ECS.h"
#include <cstddef>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../types/types.h"

#include <vulkan/vulkan_core.h>



class ImguiModule {
    public:
    	void init(void * initPtr, void * dataPtr);
	void update(void * initPtr, void * dataPtr);
        void destroy(VkDevice device);
        bool active = true;
        bool updated = false;
        ImTextureID sceneViews[MAX_FRAMES_IN_FLIGHT];
        ImVec2 previewSize = {128, 128};
        void* enginePtr;

    private:
        VkDescriptorPool imguiPool;
        void renderViewport(void * initPtr, void * dataPtr);
        void renderInstance();
        OptionalEntityID selectedInstance;
};
#endif // !IMGUI_MODULE_H
