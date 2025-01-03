#ifndef EDITOR_MODULE_H
#define EDITOR_MODULE_H 
#include "imgui.h"
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

class ImguiModule {
    public:
    	void init(void * initPtr, void * dataPtr);
	void update(void * initPtr, void * dataPtr);
        void destroy(VkDevice device);
        bool active = true;
        bool updated = false;
        ImTextureID textureID;
        ImVec2 previewSize = {128, 128};
        void* enginePtr;
    private:
        VkDescriptorPool imguiPool;
        void renderViewport(void * initPtr, void * dataPtr);
};
#endif // !IMGUI_MODULE_H
