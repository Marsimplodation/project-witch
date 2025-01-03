#ifndef SOUL_SHARD_ENINGE_H
#define SOUL_SHARD_ENINGE_H
#include "InputHandling/InputHandling.h"
#include "entt/entt.hpp"
#include "types/types.h"
#include <string>
#include <vector>

#include "Vulkan/VkRenderer.h"
struct SoulShard {
    VkRenderer::Init init;
    VkRenderer::RenderData render_data;

    std::vector<System> systems;
    entt::registry entities;
    glm::vec2 renderingResolution;
    GPUGeometry gpuGeometry;
    Camera mainCamera;
    Camera editorCamera;
    float deltaTime;
    int run();
    int startup();
    void registerSystem(void(*func)(float deltaTime), const char * name);
    void loadGeometry(std::string modelPath);
    InputHandler inputHandler;
};

#endif // !SOUL_SHARD_ENINGE_Hf
