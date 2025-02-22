#include "Scene.h"
#include "SoulShard.h"
#include "Vulkan/VkRenderer.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/geometric.hpp"
#include "types/types.h"
#include <string>


std::vector<glm::vec3> GetFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view, float nearPlane, float farPlane) {
    
    // Inverse of the combined projection and view matrix
    glm::mat4 inv = glm::inverse(proj * view);  

    
std::vector<glm::vec3> corners{
    {-1.0f,  1.0f, -1.0f},
    { 1.0f,  1.0f, -1.0f},
    { 1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f, -1.0f},
    {-1.0f,  1.0f,  1.0f},
    { 1.0f,  1.0f,  1.0f},
    { 1.0f, -1.0f,  1.0f},
    {-1.0f, -1.0f,  1.0f},
};


    // Transform each corner from NDC to world space
    // Project frustum corners into world space
    for (uint32_t j = 0; j < 8; j++) {
            glm::vec4 invCorner = inv * glm::vec4(corners[j], 1.0f);
            corners[j] = invCorner / invCorner.w;
    }

    for (uint32_t j = 0; j < 4; j++) {
            glm::vec3 dist = corners[j + 4] - corners[j];
            corners[j + 4] = corners[j] + (dist * farPlane);
            corners[j] = corners[j] + (dist * nearPlane);
    }
    return corners;
}



void Scene::updateLights() {
    SoulShard & engine = *((SoulShard*)enginePtr);
    sceneLight.direction = glm::normalize(-sceneLight.position);
    std::vector<float> cascadeSplits(SHADOW_CASCADES);
    float nearClip = 0.001f;
    float farClip = 1000.0f; //shadow max distance

    float clipRange = farClip - nearClip;

    float minZ = nearClip;
    float maxZ = nearClip + clipRange;

    float range = maxZ - minZ;
    float ratio = maxZ / minZ;

    for (uint32_t i = 0; i < SHADOW_CASCADES; i++) {
            float p = (i + 1) / static_cast<float>(SHADOW_CASCADES);
            float log = minZ * std::pow(ratio, p);
            float uniform = minZ + range * p;
            float d = 0.91f * (log - uniform) + uniform;
            cascadeSplits[i] = (d - nearClip) / clipRange;
    }
    for (int cascadeIndex = 0; cascadeIndex < SHADOW_CASCADES; cascadeIndex++) {
        float cascadeFar = cascadeSplits[cascadeIndex];
        float cascadeNear = (cascadeIndex - 1 >= 0) ? cascadeSplits[cascadeIndex - 1] : 0.0;
        std::vector<glm::vec3> frustumCorners = GetFrustumCornersWorldSpace(engine.editorCamera.projection, engine.editorCamera.view, cascadeNear, cascadeFar);

        glm::vec3 frustumCenter = glm::vec3(0.0f);
        for (uint32_t j = 0; j < 8; j++) {
                frustumCenter += frustumCorners[j];
        }
        frustumCenter /= 8.0f;

        float radius = 0.0f;
        for (uint32_t j = 0; j < 8; j++) {
                float distance = glm::length(frustumCorners[j] - frustumCenter);
                radius = glm::max(radius, distance);
        }
        radius = std::ceil(radius * 16.0f) / 16.0f;

        glm::vec3 maxExtents = glm::vec3(radius);
        glm::vec3 minExtents = -maxExtents;

        glm::vec3 lightDir = normalize(-sceneLight.position);
        sceneLight.views[cascadeIndex] = sceneLight.views[cascadeIndex] = glm::lookAt(frustumCenter - lightDir * maxExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

        sceneLight.projections[cascadeIndex] = glm::ortho(minExtents.x,
                                                          maxExtents.x,
                                                          minExtents.y, 
                                                          maxExtents.y, 
                                                          0.01f + minExtents.z, 
                                                          maxExtents.z - minExtents.z);
        sceneLight.splitDepths[cascadeIndex] = (0.001f + cascadeFar * clipRange) * -1.0f;
    }
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
