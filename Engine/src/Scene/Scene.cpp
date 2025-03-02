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
        {-1.0f,  1.0f, 0.0f},
        { 1.0f,  1.0f, 0.0f},
        { 1.0f, -1.0f, 0.0f},
        {-1.0f, -1.0f, 0.0f},
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
    std::vector<float> cascadeSplits(SHADOW_CASCADES);
    float nearClip = 0.01f;
    float farClip = 100.0f; //max shadow distance
    float lambda = 0.95f;
    //account for scene height
    //a value of 2 worked good in testing, not sure why
    float distanceFactor = (bounds.max.y - bounds.min.y) / 2; 
    
    float clipRange = farClip - nearClip;

    float minZ = nearClip;
    float maxZ = nearClip + clipRange;

    float range = maxZ - minZ;
    float ratio = maxZ / minZ;

    auto proj = glm::perspective(engine.renderer.data.editorMode ? engine.editorCamera.fov : engine.mainCamera.fov, 
					engine.renderingResolution[0] / engine.renderingResolution[1],
					nearClip,
					farClip);

    for (uint32_t i = 0; i < SHADOW_CASCADES; i++) {
            float p = (i + 1) / static_cast<float>(SHADOW_CASCADES +1);
            float log = minZ * std::pow(ratio, p);
            float uniform = minZ + range * p;
            float d = lambda * (log - uniform) + uniform;
            cascadeSplits[i] = (d - nearClip) / clipRange;
    }
    for (int cascadeIndex = 0; cascadeIndex < SHADOW_CASCADES; cascadeIndex++) {
        float cascadeFar = cascadeSplits[cascadeIndex];
        float cascadeNear = (cascadeIndex > 0) ? cascadeSplits[cascadeIndex - 1] : 0.0;
        std::vector<glm::vec3> frustumCorners = GetFrustumCornersWorldSpace(
            //switch between editor and game camera
            proj,
            engine.renderer.data.editorMode ? engine.editorCamera.view : engine.mainCamera.view,
            cascadeNear, cascadeFar);

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

        glm::vec3 maxExtents = glm::vec3(radius*2);
        glm::vec3 minExtents = -maxExtents;

        glm::vec3 lightDir = sceneLight.direction;
        sceneLight.views[cascadeIndex] = glm::lookAt(frustumCenter - lightDir * -minExtents.z * distanceFactor, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

        
        sceneLight.projections[cascadeIndex] =  glm::ortho(minExtents.x, maxExtents.x, 
                                                           minExtents.y, maxExtents.y, 
                                                           0.0f+minExtents.z*distanceFactor, 
                                                           (maxExtents.z - minExtents.z)*distanceFactor);

        sceneLight.splitDepths[cascadeIndex] = nearClip + cascadeSplits[cascadeIndex] * clipRange;

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

    geometryList.clear();
    for(auto & pair : geometry) {
        auto & info = pair.second;
        geometryList.push_back(info);
    }
    linearModels.resize(instances.size());
    modelMatrices.resize(instances.size());

    return instances.back();
}

void Scene::updateModels() {
    bounds = AABB{
        .min = glm::vec3(FLT_MAX),
        .max = glm::vec3(-FLT_MAX),
    };
    u32 iIdx = 0;
    u32 gIdx = 0;
    for(auto & info : geometryList) {
        linearModels[gIdx].indexOffset = info.indexOffset;
        linearModels[gIdx].triangleCount = info.triangleCount;
        linearModels[gIdx].instanceCount = info.instanceCount;
        for(auto & idx : info.instances) {
            auto & instance = instances[idx];
            auto & transform = registry.get<TransformComponent>(instance.entity);
            modelMatrices[iIdx++] = (transform.mat);
            auto min = glm::vec3(transform.mat * glm::vec4(info.aabb.min, 1.0f));
            auto max = glm::vec3(transform.mat * glm::vec4(info.aabb.max, 1.0f));
            bounds.min = glm::min(bounds.min, min);
            bounds.max = glm::max(bounds.max, max);
        }
        gIdx++;
    }
}
