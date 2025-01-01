#include <SoulShard.h>
#include <cstdio>
#include <glm/ext/matrix_transform.hpp>
SoulShard engine{
    .gpuGeometry = {
	.vertices = {
	    {{-0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
	    {{0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
	    {{0.5f, 0.5f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
	    {{-0.5f, 0.5f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},

	    {{-0.5f, -0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}},
	    {{0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}},
	    {{0.5f, 0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}},
	    {{-0.5f, 0.5f, -0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}}
	},
	.indices ={
	    4, 5, 6, 6, 7, 4,
	    0, 1, 2, 2, 3, 0,
	},
    }
};

void printFPS(float deltaTime) {
    printf("FPS %f\n", 1/deltaTime);
}

float fov = 45;
void updateCamera(float deltaTime) {
    engine.mainCamera.projection = glm::perspective(glm::radians(fov), engine.renderingResolution[0] / engine.renderingResolution[1], 0.1f, 10.0f);
    engine.mainCamera.projection[1][1] *= -1;
    engine.mainCamera.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); 
};

void rotateAllModels(float deltaTime) {
    auto view = engine.entities.view<Model>();
    for (auto entity : view) {
        auto& model = view.get<Model>(entity);
	for (auto & matrix : model.modelMatrices) {
	    matrix = glm::rotate(matrix, deltaTime * glm::radians(90.0f), glm::vec3(0,0,1));
	}

    }
};

int main (int argc, char *argv[]) {
    Model quadInfo = {
	.indexOffset=0,
	.triangleCount=2,
	.instanceCount=100,
	.modelMatrices = {
	},
    };
    for (int i = 0; i < 100; ++i){
	quadInfo.modelMatrices.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(0,0,1.0f/(100.0f - i))));
    }
    auto quad = engine.entities.create();
    engine.entities.emplace<Model>(quad, quadInfo);
    engine.registerSystem(updateCamera, "Game Camera");
    engine.registerSystem(rotateAllModels, "Rotation");
    //engine.registerSystem(printFPS, "FPS");
    engine.run();
}
