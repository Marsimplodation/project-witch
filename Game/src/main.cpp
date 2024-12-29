#include <SoulShard.h>
#include <cstdio>
SoulShard engine{
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
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
},
};

void printFPS(float deltaTime) {
    printf("FPS %f\n", 1/deltaTime);
}

int main (int argc, char *argv[]) {
    auto m = glm::rotate(glm::mat4(1.0f), 0.1f * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    for(auto & v : engine.vertices) {
	v.position = m * v.position;
    }
    Model quadInfo = {
	.indexOffset=0,
	.triangleCount=4,
	.instanceCount=1
    };
    auto quad = engine.entities.create();
    engine.entities.emplace<Model>(quad, quadInfo);
    engine.systems.push_back(printFPS);
    engine.run();
}
