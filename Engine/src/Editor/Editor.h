#ifndef EDITOR_MODULE_H
#define EDITOR_MODULE_H 
#include "imgui.h"
#include <cstddef>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../types/types.h"

#include <vulkan/vulkan_core.h>

struct ImGuiSerialize {
    enum Tag {
        Float,
        Vec4,
        String,
        Text
    } tag;

    std::string name; // Use a pointer for non-trivial types
    union {
        float f;
        glm::vec4 v4;
    };
};

struct ComponentSerialize {
    std::string componentName;
    std::vector<ImGuiSerialize> fields;
};

struct UIComponent {
    TypeID id;
    size_t totalSize;
    struct ComponentData {
        enum TYPE {
            VEC3,
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
        EntityID selectedInstance = 0;
};
#endif // !IMGUI_MODULE_H
