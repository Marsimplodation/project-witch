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
void rotateCamera(float deltaTime) {
    engine.mainCamera.projection = glm::perspective(glm::radians(fov), engine.renderingResolution[0] / engine.renderingResolution[1], 0.1f, 10.0f);
    engine.mainCamera.projection[1][1] *= -1;
    engine.mainCamera.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); 
};

float elapsedTime = 0.0f;
void rotateAllModels(float deltaTime) {
    elapsedTime += deltaTime;
    auto view = engine.entities.view<Model>();
    for (auto entity : view) {
        auto& model = view.get<Model>(entity);
	model.modelMatrices[0] = glm::rotate(glm::mat4(1.0f), elapsedTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model.modelMatrices[1] = glm::rotate(glm::mat4(1.0f), -elapsedTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    }
};

int main (int argc, char *argv[]) {
    Model quadInfo = {
	.indexOffset=0,
	.triangleCount=2,
	.instanceCount=2,
	.modelMatrices = {
	    glm::mat4(1.0f),
	    glm::translate(glm::mat4(1.0f), glm::vec3(0,0,1)),
	},
    };
    auto quad = engine.entities.create();
    engine.entities.emplace<Model>(quad, quadInfo);
    engine.systems.push_back(rotateCamera);
    engine.systems.push_back(rotateAllModels);
    //engine.systems.push_back(printFPS);
    engine.run();
}
