#include "Physics/JoltImpl.h"
#include "Scene/Scene.h"
#include "entt/entity/fwd.hpp"
#include "glm/fwd.hpp"
#include "glm/trigonometric.hpp"
#include "types/types.h"
#include <SoulShard.h>
#include <climits>
#include <cstdio>
#include <glm/ext/matrix_transform.hpp>
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
    engine.mainCamera.projection = glm::perspective(
					glm::radians(fov),
					engine.renderingResolution[0] / engine.renderingResolution[1],
					0.1f, 1000.0f);
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
void spawnModels(float deltaTime) {
    if(engine.scene.instanceCount >= 400) return;
    float xi1 = (2*((float)rand()/(float)INT_MAX)-1.0f) * 20;
    float xi2 = (((float)rand()/(float)INT_MAX)) * 20;
    float xi3 = (2*((float)rand()/(float)INT_MAX)-1.0f) * 20;
    auto dir = glm::vec3(xi1, xi2, xi3);
    auto & cube = engine.scene.instantiateModel("Cube", "Cube 1");
    createRigidBody(cube, engine.scene);
    auto & trans = engine.scene.registry.get<TransformComponent>(cube.entity);
    trans.mat = glm::translate(trans.mat, dir); 
    AddLinearVelocity(cube, Vec3(-xi1, xi2, -xi3),engine.scene);
};
void rotateModels(float deltaTime) {
    auto view = engine.scene.registry.view<TransformComponent>();
    for (auto & entity : view) {
	auto & trans = engine.scene.registry.get<TransformComponent>(entity);
	trans.mat = glm::rotate(trans.mat, deltaTime * glm::radians(180.0f), glm::normalize(glm::vec3(0,0.0,1.0))); 
    }
};

int main (int argc, char *argv[]) {
    engine.startup();
    engine.loadGeometry("../Game/Assets/test.obj");
    engine.registerSystem(updateCamera, "Game Camera");
    engine.registerSystem(spawnModels, "spawn Models");
    engine.systems.back().active = false;
    //engine.registerSystem(rotateModels, "Rotation");
    //engine.registerSystem(printFPS, "FPS");
    engine.run();
}
