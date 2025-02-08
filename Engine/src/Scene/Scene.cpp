#include "Scene.h"
#include "types/types.h"
#include <string>

void Scene::initScene() {
    sceneLight.projection[1][1] *= -1;
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
