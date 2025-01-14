#ifndef ENGINE_SCENE
#define ENGINE_SCENE
#include "../types/types.h"
#include "glm/fwd.hpp"
#include <unordered_map>

const int MAX_INSTANCES = 10000;
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};
struct GeometryInfo {
    AABB aabb;
    bool active;
    u32 indexOffset;
    u32 triangleCount;
    u32 instanceCount;
    std::vector<glm::mat4> modelMatrices;
};

struct Instance {
    std::string name;
    GeometryInfo & instanceOf;
    u32 transformIdx;
};

struct Scene {
    std::vector<glm::mat4> modelMatrices;
    std::vector<Instance> instances;
    std::vector<Model> linearModels;
    std::unordered_map<std::string, GeometryInfo> geometry;
    Instance & instantiateModel(std::string objName, std::string instanceName);
    void translateInstance(glm::vec3 & dir, Instance & instance);
    u32 instanceCount = 0;
    void updateModels();

private:
};


#endif // !ENGINE_SCENE
