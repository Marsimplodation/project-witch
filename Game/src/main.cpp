#include "Scene/Scene.h"
#include "glm/fwd.hpp"
#include "glm/trigonometric.hpp"
#include "types/ECS.h"
#include "types/types.h"
#include <SoulShard.h>
#include <climits>
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
    engine.mainCamera.far=100.0f;
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
struct Test {float a;};
void translateModels(float deltaTime) {
    for(auto & instance:engine.scene.instances) {
        auto * tpr = engine.scene.registry.getComponent<TransformComponent>(instance.entity);
        auto * testpr = engine.scene.registry.getComponent<Test>(instance.entity);
        if(!tpr || !testpr) continue;
        tpr->mat = glm::translate(tpr->mat, glm::vec3(0,testpr->a * deltaTime, 0));
    }
};


int main (int argc, char *argv[]) {
    //ECS_BENCHMARK();
    engine.startup();

    auto & gui = engine.renderer.data.gui;
    ECS::registerType<Test>();
    UIComponent TestUI {
        .id = ECS::getTypeIndex<Test>(),
        .totalSize = ECS::getTotalTypeSize<Test>(),
        .name = "Test",
        .data =  {
            UIComponent::ComponentData {
                .type = UIComponent::ComponentData::TYPE::FLOAT,
                .offset = 0,
                .name = "a",
            },
        },

    };
    gui.registeredComponents.push_back(TestUI);


    engine.loadGeometry("../Game/Assets/Sponza/sponza.obj");
    //engine.loadGeometry("../Game/Assets/cube.obj");
    //engine.loadGeometry("../Game/Assets/cube.obj");
    engine.registerSystem(updateCamera, "Game Camera");
    engine.registerSystem(translateModels, "Translate Models");

    engine.run();
}
