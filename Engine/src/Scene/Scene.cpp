#include <cstring>
#define ECS_IMPLEMENTATION
#include "Scene.h"
#include "SoulShard.h"
#include "Vulkan/VkRenderer.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "types/defines.h"


#include "types/types.h"
#include <regex>
#include <string>

void Scene::initScene(){
    SoulShard & engine = *((SoulShard*)enginePtr);
    auto & gui = engine.renderer.data.gui;
    registry = ECS();
    registry.registerType<TransformComponent>();
    registry.registerType<AABB>();
    registry.registerType<InstanceName>();
    registry.registerType<PointLight>();
    UIComponent AABBUI {
        .id = ECS::getTypeIndex<AABB>(),
        .totalSize = ECS::getTotalTypeSize<AABB>(),
        .name = "AABB",
        .data =  {
            UIComponent::ComponentData {
                .type = UIComponent::ComponentData::TYPE::VEC3,
                .offset = 0,
                .name = "Min",
            },
            UIComponent::ComponentData {
                .type = UIComponent::ComponentData::TYPE::VEC3,
                .offset = sizeof(glm::vec3),
                .name = "Max",
            }
        },

    };
    UIComponent PointLightUI {
        .id = ECS::getTypeIndex<PointLight>(),
        .totalSize = ECS::getTotalTypeSize<PointLight>(),
        .name = "Point Light",
        .data =  {
            UIComponent::ComponentData {
                .type = UIComponent::ComponentData::TYPE::COLOR,
                .offset = sizeof(glm::vec4),
                .name = "Color",
            },
            UIComponent::ComponentData {
                .type = UIComponent::ComponentData::TYPE::FLOAT,
                .offset = sizeof(glm::vec4) + sizeof(glm::vec3),
                .name = "Intensity",
            },
        },
    };
    gui.registeredComponents.push_back(AABBUI);
    gui.registeredComponents.push_back(PointLightUI);
}

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

void Scene::createPointLight() {
    std::string instanceName = "Point Light";
    Instance instance = {
        .entity = registry.newEntity(),
    };
    size_t len = instanceName.size() > 255 ? 254 : instanceName.size();
    InstanceName name;
    name.name[255] = '0';
    memcpy(name.name, "Point Light", len); 
    TransformComponent transform = {
        glm::mat4(1.0)
    };
    registry.addComponent<TransformComponent>(instance.entity, transform);
    registry.addComponent<PointLight>(instance.entity, 
                                      PointLight{
                                        glm::vec4(1,1,1,0.1f),
                                        glm::vec4(1,1,1,1.0f),
                                      });
    registry.addComponent<InstanceName>(instance.entity, name);

    instances.push_back(instance);
}


void Scene::updateLights() {
    SoulShard & engine = *((SoulShard*)enginePtr);

    auto pLightIdx = 0;
    for(auto instance: instances) {
        auto transformPtr = registry.getComponent<TransformComponent>(instance.entity);
        auto light = registry.getComponent<PointLight>(instance.entity);
        if(!light || !transformPtr) continue;
        auto radius = light->position[3];
        light->position[3] = 1.0f;
        light->position = transformPtr->mat[3];
        pointLights[pLightIdx++] = *light;
    }
    for(int i = pLightIdx; i < pointLights.size(); ++i)
        pointLights[i] = PointLight{glm::vec4(0.0f), glm::vec4(0.0f)};


    std::vector<float> cascadeSplits(SHADOW_CASCADES);
    float nearClip = 0.1f;
    float farClip = 100.0f; //max shadow distance
    float lambda = 0.95f;
    //account for scene height
    float distanceFactor = bounds.max.y;
    
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
            float p = (i + 1) / static_cast<float>(SHADOW_CASCADES);
            float log = minZ * std::pow(ratio, p);
            float uniform = minZ + range * p;
            float d = lambda * (log - uniform) + uniform;
            cascadeSplits[i] = (d - nearClip) / clipRange;
    }
    for (int cascadeIndex = 0; cascadeIndex < SHADOW_CASCADES; cascadeIndex++) {
        float cascadeFar = cascadeSplits[cascadeIndex];
        float cascadeNear = (cascadeIndex > 0) ? cascadeSplits[cascadeIndex - 1] : 0.0;
        std::vector<glm::vec3> frustumCorners = GetFrustumCornersWorldSpace(
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
        glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z*distanceFactor, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f + minExtents.z * distanceFactor, (maxExtents.z - minExtents.z)*distanceFactor);
        //shift down distance factor for detailed far maps
        //as they are further away they don't need to be shifted back as much
        distanceFactor /= 2;
        distanceFactor = fmaxf(distanceFactor, 1.0f);
        sceneLight.views[cascadeIndex] = lightViewMatrix; 

        
        sceneLight.projections[cascadeIndex] = lightOrthoMatrix; 

        sceneLight.splitDepths[cascadeIndex] = nearClip + cascadeSplits[cascadeIndex] * clipRange;

    }
};

Instance & Scene::instantiateModel(std::string objName, std::string instanceName) {
    if(++instanceCount >= MAX_INSTANCES) {
        printf("too many instances\n");
        exit(1);
    }
    auto modelIdx = geometry[objName];
    auto & model = geometryList[modelIdx];
    model.instanceCount++;
    model.instances.push_back(instances.size());

    Instance instance = {
        .entity = registry.newEntity(),
    };
    size_t len = instanceName.size() > 255 ? 254 : instanceName.size();
    InstanceName name;
    name.name[255] = '0';
    memcpy(name.name, instanceName.c_str(), len); 
    TransformComponent transform = {
        glm::mat4(1.0)
    };
    registry.addComponent<TransformComponent>(instance.entity, transform);
    registry.addComponent<AABB>(instance.entity, model.aabb);
    registry.addComponent<InstanceName>(instance.entity, name);

    instances.push_back(instance);
    return instances.back();
}

struct Plane {
    glm::vec3 normal;
    float d;

    float distance(const glm::vec3& point) const {
        return glm::dot(normal, point) + d;
    }
};

inline void extractFrustumPlanes(Plane planes[6], const glm::mat4& VP) {
    for (int i = 0; i < 6; i++) {
        int sign = (i % 2 == 0) ? 1 : -1;
        int row = i / 2;

        planes[i].normal.x = VP[0][3] + sign * VP[0][row];
        planes[i].normal.y = VP[1][3] + sign * VP[1][row];
        planes[i].normal.z = VP[2][3] + sign * VP[2][row];
        planes[i].d        = VP[3][3] + sign * VP[3][row];

        float len = glm::length(planes[i].normal);
        planes[i].normal /= len;
        planes[i].d /= len;
    }
}

inline bool isAABBInFrustum(const AABB& aabb, const Plane planes[6]) {
    for (int i = 0; i < 6; i++) {
        glm::vec3 p(
            (planes[i].normal.x > 0) ? aabb.max.x : aabb.min.x,
            (planes[i].normal.y > 0) ? aabb.max.y : aabb.min.y,
            (planes[i].normal.z > 0) ? aabb.max.z : aabb.min.z
        );

        if (planes[i].distance(p) < 0) {
            return false; // AABB is outside this plane
        }
    }
    return true; // AABB is inside or intersecting the frustum
}


void Scene::updateModels() {
    SoulShard & engine = *((SoulShard*)enginePtr);
    _bounds = AABB{
        .min = glm::vec3(FLT_MAX),
        .max = glm::vec3(-FLT_MAX),
    };
    for(int i = 0; i < 1+SHADOW_CASCADES; ++i){
        _linearModels[i].clear();
        _modelMatrices[i].clear();
    }
    _matrixOffsets.clear();
    //camera
    auto proj = engine.renderer.data.editorMode ? engine.editorCamera.projection : engine.mainCamera.projection;
    proj[1][1] *= -1;
    auto view = engine.renderer.data.editorMode ? engine.editorCamera.view : engine.mainCamera.view;

    std::array<glm::mat4, SHADOW_CASCADES + 1> viewsProjs{
        proj * view,
    };
    for(int c = 0; c < SHADOW_CASCADES; ++c)
        viewsProjs[c+1] = sceneLight.projections[c]*sceneLight.views[c];

    Plane planes[SHADOW_CASCADES + 1][6];
    for(int c = 0; c < SHADOW_CASCADES + 1; ++c)
        extractFrustumPlanes(planes[c], viewsProjs[c]);
    for(auto & info : geometryList) {
        u32 instanceCount[SHADOW_CASCADES + 1];
        for(int c = 0; c < SHADOW_CASCADES + 1; ++c){
            instanceCount[c] = 0;
        }
        for(auto & idx : info.instances) {
            auto & instance = instances[idx];
            auto transformPtr = registry.getComponent<TransformComponent>(instance.entity);
            auto aabb = registry.getComponent<AABB>(instance.entity);
            if(!transformPtr) continue;
            if(!aabb) continue;
            auto & transform = *transformPtr;
            //---- update AABB / Update Scene Bounds ----//
            glm::vec3 corners[8] = {
                {info.aabb.min.x, info.aabb.min.y, info.aabb.min.z},
                {info.aabb.max.x, info.aabb.min.y, info.aabb.min.z},
                {info.aabb.min.x, info.aabb.max.y, info.aabb.min.z},
                {info.aabb.max.x, info.aabb.max.y, info.aabb.min.z},
                {info.aabb.min.x, info.aabb.min.y, info.aabb.max.z},
                {info.aabb.max.x, info.aabb.min.y, info.aabb.max.z},
                {info.aabb.min.x, info.aabb.max.y, info.aabb.max.z},
                {info.aabb.max.x, info.aabb.max.y, info.aabb.max.z}
            };

            glm::vec3 newMin(FLT_MAX), newMax(-FLT_MAX);

            for (int i = 0; i < 8; ++i) {
                glm::vec3 transformed = glm::vec3(transform.mat * glm::vec4(corners[i], 1.0f));
                newMin = glm::min(newMin, transformed);
                newMax = glm::max(newMax, transformed);
            }

            aabb->min = newMin;
            aabb->max = newMax;
            _bounds.min = glm::min(_bounds.min, aabb->min);
            _bounds.max = glm::max(_bounds.max, aabb->max);

            //---- frustum culling ----//
            for(int c = 0; c < SHADOW_CASCADES + 1; ++c){
                if(frustumCulling && !isAABBInFrustum(*aabb, planes[c])) continue;
                _modelMatrices[c].push_back(transform.mat);
                instanceCount[c]++;
            }
        }
        //--- Push Models for rendering ---//
        for(int c = 0; c < SHADOW_CASCADES + 1; ++c){
            _linearModels[c].push_back({
                .indexOffset=info.indexOffset,
                .triangleCount=info.triangleCount,
                .instanceCount=instanceCount[c]});
        }
    }
}
void Scene::pushUpdatedModels() {
    SoulShard & engine = *((SoulShard*)enginePtr);
    auto frame = (engine.renderer.data.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    for(int i = 0; i < 1+SHADOW_CASCADES; ++i){
        linearModels[frame][i].resize(_linearModels[i].size());
        modelMatrices[frame][i].resize(_modelMatrices[i].size());
    }
    bounds = _bounds;
    for(int i = 0; i < 1+SHADOW_CASCADES; ++i){
        for(int j = 0; j < _linearModels[i].size(); ++j)
            linearModels[frame][i][j] = _linearModels[i][j];
        
        for(int j = 0; j < _modelMatrices[i].size(); ++j)
            modelMatrices[frame][i][j] = _modelMatrices[i][j];
    }
}
