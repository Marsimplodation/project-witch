#include "entt/entity/fwd.hpp"
#include "types/types.h"
#include <SoulShard.h>
#include <cstdio>
#include <glm/ext/matrix_transform.hpp>
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
    engine.mainCamera.projection = glm::perspective(
					glm::radians(fov),
					engine.renderingResolution[0] / engine.renderingResolution[1],
					0.1f, 1000.0f);
    engine.mainCamera.projection[1][1] *= -1;
    engine.mainCamera.view = glm::lookAt(position,
				    position + forward,
				    up); 
};

float elapsedTime = 0.0f;
void rotateAllModels(float deltaTime) {
    auto view = engine.entities.view<Model>();
    elapsedTime += deltaTime;
    entt::entity player;
    for (auto entity : view) {
    auto& model = view.get<Model>(entity);
	if(model.name != "sponza_01") continue;
    for (auto & matrix : model.modelMatrices) {
	matrix = glm::translate(matrix,
		      sinf(elapsedTime) * up * 0.001f);
    }
	return;;
    }

    for (auto entity : view) {
        auto& model = view.get<Model>(entity);
	for (auto & matrix : model.modelMatrices) {
	    matrix = glm::rotate(matrix,
			  deltaTime * glm::radians(80.0f),
			  glm::normalize(glm::vec3(0,0.5,1)));
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
