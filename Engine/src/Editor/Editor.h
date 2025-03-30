#ifndef EDITOR_MODULE_H
#define EDITOR_MODULE_H 
#include "imgui.h"
#include "types/ECS.h"
#include <cstddef>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../types/types.h"

#include <vulkan/vulkan_core.h>

struct UIComponent {
    TypeID id;
    size_t totalSize;
    struct ComponentData {
        enum TYPE {
            VEC3,
            COLOR,
            FLOAT
        } type;
        u32 offset;
        std::string name;
    };
    std::string name;
    std::vector<ComponentData> data;
};

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

        std::vector<UIComponent> registeredComponents;
    private:
        VkDescriptorPool imguiPool;
        void renderViewport(void * initPtr, void * dataPtr);
        void renderInstance();
        OptionalEntityID selectedInstance;
};
#endif // !IMGUI_MODULE_H
