#include "entt/entity/fwd.hpp"
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

float fov = 45;
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
std::vector<glm::vec3> rotations = {{0,1,0}};
void rotateAllModels(float deltaTime) {
    auto view = engine.entities.view<Model>();
    elapsedTime += deltaTime;
    entt::entity player;
    for (auto entity : view) {
	auto& model = view.get<Model>(entity);
	if(model.instanceCount >= 2000) break;
	model.instanceCount++;
	auto mat = glm::mat4(1.0);
	float xi1 = (2*((float)rand()/(float)INT_MAX)-1.0f) * 20;
	float xi2 = (2*((float)rand()/(float)INT_MAX)-1.0f) * 20;
	float xi3 = (2*((float)rand()/(float)INT_MAX)-1.0f) * 20;
	mat = glm::translate(mat, glm::vec3(xi1, xi2, xi3));
	model.modelMatrices.push_back(mat);
	xi1 = (2*((float)rand()/(float)INT_MAX)-1.0f);
	xi2 = (2*((float)rand()/(float)INT_MAX)-1.0f);
	xi3 = (2*((float)rand()/(float)INT_MAX)-1.0f);
	rotations.push_back({xi1, xi2, xi3});
    }
    int i = 0;
    for (auto entity : view) {
        auto& model = view.get<Model>(entity);
	for (auto & matrix : model.modelMatrices) {
	    matrix = glm::translate(matrix,
			  sinf(elapsedTime) * 0.0001f *  glm::normalize(glm::vec3(rotations[i++])));
	}

    }
};

int main (int argc, char *argv[]) {
    engine.startup();
    engine.loadGeometry("../Game/Assets/test.obj");
    engine.registerSystem(updateCamera, "Game Camera");
    engine.registerSystem(rotateAllModels, "Rotation");
    //engine.registerSystem(printFPS, "FPS");
    engine.run();
}
