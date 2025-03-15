#ifndef SOUL_SHARD_ENINGE_H
#define SOUL_SHARD_ENINGE_H
#include "InputHandling/InputHandling.h"
#include "Scene/Scene.h"
#include "types/types.h"
#include <string>
#include <vector>

#include "Vulkan/VkRenderer.h"
struct SoulShard {
    VkRenderer renderer;
    std::vector<System> systems;
    glm::vec2 renderingResolution;
    GPUGeometry gpuGeometry;
    Camera mainCamera;
    Camera editorCamera;
    Scene scene;
    float deltaTime;
    int run();
    int startup();
    void registerSystem(void(*func)(float deltaTime), const char * name);
    void loadGeometry(std::string modelPath);
    void loadScene(const std::string & file);
    void destroy();
    InputHandler inputHandler;
};

#endif // !SOUL_SHARD_ENINGE_Hf
