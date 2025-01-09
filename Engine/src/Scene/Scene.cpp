#include "Scene.h"
#include <string>

Instance & Scene::instantiateModel(std::string objName, std::string instanceName) {
    if(++instanceCount >= MAX_INSTANCES) {
        printf("too many instances\n");
        exit(1);
    }
    auto & model = geometry[objName];
    model.instanceCount++;
    model.modelMatrices.push_back(glm::mat4(1.0f));

    Instance instance = {
        .name = instanceName,
        .instanceOf = model,
        .transformIdx = model.instanceCount - 1,
    };

    instances.push_back(instance);

    linearModels.clear();
    modelMatrices.clear();
    for(auto & pair : geometry) {
        auto & info = pair.second;
        linearModels.push_back({
            .indexOffset = info.indexOffset,
            .triangleCount = info.triangleCount,
            .instanceCount = info.instanceCount,
        });
        for(auto & matrix : info.modelMatrices) {
            modelMatrices.push_back(matrix);
        }
    }
    return instances.back();
}

void Scene::translateInstance(glm::vec3 & dir, Instance & instance) {
    auto & mat = instance.instanceOf.modelMatrices[instance.transformIdx];
    mat = glm::translate(mat, dir);
    updateModels();
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
        for(auto & matrix : info.modelMatrices) {
            modelMatrices.push_back(matrix);
        }
    }
}
