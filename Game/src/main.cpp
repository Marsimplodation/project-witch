#include "Scene/Scene.h"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/fwd.hpp"
#include "glm/trigonometric.hpp"
#include "types/ECS.h"
#include "types/ECS_UI.h"
#include "types/types.h"
#include <SoulShard.h>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <vector>
SoulShard engine{};
auto & input = engine.inputHandler;
void printFPS(float deltaTime) {
    printf("FPS %f\n", 1/deltaTime);
}

float fov = 70;
glm::vec3 position = glm::vec3(0,0,15.0f);
glm::vec3 forward = glm::vec3(0,0,-1.0f);
glm::vec3 up = glm::vec3(0,1,0.0f);
glm::vec3 right = glm::vec3(1,0,0.0f);


float yaw = 0.0f;
float pitch = 0.0f;
void updateCamera(float deltaTime) {
    if(engine.renderer.data.editorMode) return;
    engine.mainCamera.near=0.1f;
    engine.mainCamera.far=1000.0f;
    engine.mainCamera.fov=glm::radians(fov);
    engine.mainCamera.projection = glm::perspective(
					engine.mainCamera.fov,
					engine.renderingResolution[0] / engine.renderingResolution[1],
					engine.mainCamera.near,
					engine.mainCamera.far);
    engine.mainCamera.projection[1][1] *= -1;
    engine.mainCamera.view = glm::lookAt(position,
                                    position + forward,
                                    up); 
    input.captureMouse();
    auto delta = input.getMouseDelta();
    up = glm::vec3(engine.mainCamera.view[1]);
    right = glm::vec3(engine.mainCamera.view[0]);
    forward = glm::vec3(engine.mainCamera.view[2]);
    yaw -=  delta[0] * deltaTime * 40.0f;
    pitch -= delta[1] * deltaTime * 40.0f;
    auto rot = createRotationMatrix(yaw, pitch, 0.0f);
    glm::vec3 f = {0,0,-1};
    glm::vec3 u = {0,1,0};
    glm::vec3 r = {1,0,0};
    forward = rot * f;
    up = rot * u;
    right = rot * r;
    if(input.isKeyPressed(KEY_W)) position += forward * deltaTime * 10.0f;
    if(input.isKeyPressed(KEY_S)) position -= forward * deltaTime * 10.0f;
    if(input.isKeyPressed(KEY_D)) position += right * deltaTime * 10.0f;
    if(input.isKeyPressed(KEY_A)) position -= right * deltaTime * 10.0f;
};

float elapsedTime = 0.0f;
u32 spawned = 0;
struct TestResult {bool collides; glm::vec3 normal; float penetration;};


TestResult checkIfCollides(const AABB& a, const AABB& b) {
    TestResult result{false, glm::vec3(0.0f), 0.0f};

    // Compute the half extents and centers
    glm::vec3 aCenter = (a.min + a.max) * 0.5f;
    glm::vec3 bCenter = (b.min + b.max) * 0.5f;
    glm::vec3 aHalf = (a.max - a.min) * 0.5f;
    glm::vec3 bHalf = (b.max - b.min) * 0.5f;

    // Compute delta and overlap on each axis
    glm::vec3 delta = bCenter - aCenter;
    glm::vec3 overlap = aHalf + bHalf - glm::abs(delta);

    // If there's no overlap on any axis, return no collision
    if (overlap.x <= 0.0f || overlap.y <= 0.0f || overlap.z <= 0.0f)
        return result;

    result.collides = true;

    // Find the axis of minimum penetration
    if (overlap.x < overlap.y && overlap.x < overlap.z) {
        result.normal = glm::vec3((delta.x < 0 ? -1.0f : 1.0f), 0.0f, 0.0f);
        result.penetration = overlap.x;
    } else if (overlap.y < overlap.z) {
        result.normal = glm::vec3(0.0f, (delta.y < 0 ? -1.0f : 1.0f), 0.0f);
        result.penetration = overlap.y;
    } else {
        result.normal = glm::vec3(0.0f, 0.0f, (delta.z < 0 ? -1.0f : 1.0f));
        result.penetration = overlap.z;
    }

    return result;
}


struct Physics {glm::vec3 velocity; float mass; bool isStatic;};
void physicsSystem(float deltaTime) {
    const static auto instances = engine.scene.registry.allEntitiesWith<TransformComponent, AABB, Physics>();
    for(int i = 0; i < instances.size(); ++i) {
        auto & instance = instances[i];
        auto refs = ECS::getMultipleComponents<TransformComponent, Physics, AABB>(instance);
        if(!refs) continue;
        auto [iTransform, iPhysics, iAABB] = refs.unwrap();

        glm::vec3 position = glm::vec3(iTransform.mat[3]);
        position += iPhysics.velocity * deltaTime;
        iPhysics.velocity.y -= 9.18f * deltaTime;


        for(int j = 0; j < instances.size(); ++j) {
            if(i == j) continue;
            auto & other = instances[j];
            auto refs = ECS::getMultipleComponents<TransformComponent, Physics, AABB>(other);
            if(!refs) continue;
            auto [oTransform, oPhysics, oAABB] = refs.unwrap();

            auto result = checkIfCollides(oAABB, iAABB); 
            if(!result.collides) continue;
            glm::vec3 correction = result.normal * result.penetration;
            position += correction;


            auto normal = result.normal; 
            auto relativeVelocity = oPhysics.velocity - iPhysics.velocity; // other is static
            float relativeNormal = glm::dot(normal, relativeVelocity);
            if (relativeNormal >= 0.0f) {
                float e = 0.5f; // restitution
                float invMassA = 1.0f/iPhysics.mass;
                float invMassB = 1.0f/oPhysics.mass;
                float impulseMag = (1.0f + e) * relativeNormal / (invMassA + invMassB);
                auto impulse = impulseMag * normal;
                iPhysics.velocity += impulse * invMassA;
            }
        }
        if(iPhysics.isStatic) iPhysics.velocity = glm::vec3(0.0f);
        else iTransform.mat[3] = glm::vec4(position, 1.0f);
    }
};

int main (int argc, char *argv[]) {
    //ECS_BENCHMARK();
    engine.startup();

    auto & gui = engine.renderer.data.gui;
    ECS::registerType<Physics>();
    ECS_UI::registerType<Physics>("Physics");
    ECS_UI::addToType<Physics>("Velocity", offsetof(Physics, velocity), ECS_UI::ELEMENT_TYPE::VEC3);
    ECS_UI::addToType<Physics>("Mass", offsetof(Physics, mass), ECS_UI::ELEMENT_TYPE::FLOAT);
    ECS_UI::addToType<Physics>("isStatic", offsetof(Physics, isStatic), ECS_UI::ELEMENT_TYPE::BOOL);


    engine.loadGeometry("../Game/Assets/cube.obj");
    #define rnd (2.0f*((float)std::rand() / (float) RAND_MAX)-1.0f) * 10.0f
    auto data = Physics{{}, 1.0f, false};
        data.mass = 1.0f;

    for(int i = 0; i < 100; ++i) {
        auto & instance = engine.scene.instantiateModel("Cube", std::format("Cube {}", i));
        engine.scene.registry.addComponent<Physics>(instance.entity, data);
        auto tpr = engine.scene.registry.getComponent<TransformComponent>(instance.entity);
        auto pptr = engine.scene.registry.getComponent<Physics>(instance.entity);
        glm::vec3 pos{rnd, rnd, rnd};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        tpr->mat = glm::scale(glm::mat4(1.0f), scale); 
        tpr->mat = glm::translate(tpr->mat, pos); 
    }
    data.isStatic = true;
    data.mass = 10000.0f;
    auto & floor = engine.scene.instances[0];
    engine.scene.registry.addComponent<Physics>(floor.entity, data);
    auto tpr = engine.scene.registry.getComponent<TransformComponent>(floor.entity);
    auto pptr = engine.scene.registry.getComponent<Physics>(floor.entity);
    glm::vec3 scale{100.0f, 1.0f, 100.0f};
    glm::vec3 pos{0.0f, -5.1f, 0.0f};
    tpr->mat = glm::scale(glm::mat4(1.0f), scale); 
    tpr->mat = glm::translate(tpr->mat, pos); 

    engine.registerSystem(updateCamera, "Game Camera");
    engine.registerSystem(physicsSystem, "Physics");
    engine.systems.back().active = false;

    engine.run();
}
