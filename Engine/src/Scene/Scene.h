#ifndef ENGINE_SCENE
#define ENGINE_SCENE
#include "../types/types.h"
#include "Vulkan/VkRenderer.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "types/ECS.h"
#include "types/defines.h"
#include <unordered_map>

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};
struct Instance {
    std::string name;
    EntityID entity;
    AABB aabb;
};

struct GeometryInfo {
    AABB aabb;
    bool active;
    u32 indexOffset;
    u32 triangleCount;
    u32 instanceCount;
    std::vector<u32> instances;
};


struct Scene {
    AABB bounds;
    std::vector<glm::mat4> modelMatrices;
    std::vector<u32> matrixOffsets;
    std::vector<Instance> instances;
    std::vector<Model> linearModels[1 + SHADOW_CASCADES];
    std::vector<PointLight> pointLights;
    ECS registry;

    DirectionLight sceneLight = DirectionLight{
        .position = glm::vec4(4,1.5,4,0),
        .direction = glm::vec4(-1,-1,-1,0),
        .color = glm::vec4(1.0f),
        .debugFactors=glm::vec4(1.0f),
    };
    std::unordered_map<std::string, u32> geometry;
    std::vector<GeometryInfo> geometryList;
    Instance & instantiateModel(std::string objName, std::string instanceName);
    u32 instanceCount = 0;
    void updateModels();
    void pushUpdatedModels();
    void updateLights();
    void initScene();
    void *enginePtr;

private:
    std::vector<glm::mat4> _modelMatrices;
    std::vector<u32> _matrixOffsets;
    std::vector<Model> _linearModels[1 + SHADOW_CASCADES];
    AABB _bounds;
};


#endif // !ENGINE_SCENE
