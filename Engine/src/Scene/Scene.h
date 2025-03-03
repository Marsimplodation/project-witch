#ifndef ENGINE_SCENE
#define ENGINE_SCENE
#include "../types/types.h"
#include "Vulkan/VkRenderer.h"
#include "entt/entt.hpp"
#include "entt/entity/fwd.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <unordered_map>

const int MAX_INSTANCES = 10000;
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};
struct Instance {
    std::string name;
    entt::entity entity;
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
    std::vector<Instance> instances;
    std::vector<Model> linearModels;
    std::vector<PointLight> pointLights;
    entt::registry registry;

    DirectionLight sceneLight = DirectionLight{
        .position = glm::vec4(4,1.5,4,0),
        .direction = glm::vec4(-1,-1,-1,0),
        .color = glm::vec4(1.0f),
        .debugFactors=glm::vec4(1.0f),
    };
    std::unordered_map<std::string, GeometryInfo> geometry;
    std::vector<GeometryInfo> geometryList;
    Instance & instantiateModel(std::string objName, std::string instanceName);
    u32 instanceCount = 0;
    void updateModels();
    void updateLights();
    void initScene();
    void *enginePtr;

private:
};


#endif // !ENGINE_SCENE
