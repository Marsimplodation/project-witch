#include "Scene.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "types/types.h"
#include <string>

void Scene::updateLights() {
    sceneLight.direction = glm::normalize(-sceneLight.position);
    sceneLight.view = glm::lookAt(glm::vec3(sceneLight.position),
                                  glm::vec3(sceneLight.position + sceneLight.direction), glm::vec3(0,1,0)); 


    
    float near_plane = 0.01f, far_plane = glm::distance(sceneLight.position, glm::vec4(0.0f)) * 1.1f;
    sceneLight.projection = glm::ortho(-20.0f, 20.0f, 20.0f, -20.0f, near_plane, far_plane);
};

Instance & Scene::instantiateModel(std::string objName, std::string instanceName) {
    if(++instanceCount >= MAX_INSTANCES) {
        printf("too many instances\n");
        exit(1);
    }
    auto & model = geometry[objName];
    model.instanceCount++;
    model.instances.push_back(instances.size());

    Instance instance = {
        .name = instanceName,
        .entity = registry.create(),
    };
    TransformComponent transform = {
        glm::mat4(1.0)
    };
    registry.emplace<TransformComponent>(instance.entity, transform);

    instances.push_back(instance);
    return instances.back();
}

void Scene::updateModels() {
    linearModels.clear();
    modelMatrices.clear();
    for(auto & pair : geometry) {
        auto & info = pair.second;
        linearModels.push_back({
            .indexOffset = info.indexOffset,
            .triangleCount = info.triangleCount,
            .instanceCount = info.instanceCount,
        });
        for(auto & iIdx : info.instances) {
            auto & instance = instances[iIdx];
            auto & transform = registry.get<TransformComponent>(instance.entity);
            modelMatrices.push_back(transform.mat);
        }
    }
}
