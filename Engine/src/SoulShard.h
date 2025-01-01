#ifndef SOUL_SHARD_ENINGE_H
#define SOUL_SHARD_ENINGE_H
#include "entt/entt.hpp"
#include "types/types.h"
#include <string>
#include <vector>

struct System{
    bool active;
    void(*func)(float deltaTime);
    std::string name;
};

struct SoulShard {
    std::vector<System> systems;
    entt::registry entities;
    glm::vec2 renderingResolution;
    GPUGeometry gpuGeometry;
    Camera mainCamera;
    int run();
    void registerSystem(void(*func)(float deltaTime), const char * name);
};

#endif // !SOUL_SHARD_ENINGE_Hf
