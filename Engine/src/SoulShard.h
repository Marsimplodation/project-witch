#ifndef SOUL_SHARD_ENINGE_H
#define SOUL_SHARD_ENINGE_H
#include "entt/entt.hpp"
#include "types/types.h"
#include <vector>
struct SoulShard {
    std::vector<void(*)(float deltaTime)> systems;
    entt::registry entities;
    glm::vec2 renderingResolution;
    GPUGeometry gpuGeometry;
    Camera mainCamera;
    int run();
};

#endif // !SOUL_SHARD_ENINGE_Hf
