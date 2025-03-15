#include "types/types.h"
#include <iostream>
    // Sample component
struct Position {
    float x, y;
};
struct Test {
    float x, y, z;
};

struct Transform {
    float x, y, z;
};

struct Velocity {
    float dx, dy, dz;
};

void ECS_BENCHMARK() {
    printf("----- ECS BENCHMARK -------\n");
    ECS ecs;
    ecs.registerType<Position>();
    ecs.registerType<Test>();
    ecs.registerType<Velocity>();
    ecs.registerType<Transform>();

    EntityID entity1 = ecs.newEntity();
    EntityID entity2 = ecs.newEntity();

    bool hasComponent = false;
    // Add components
    ecs.addComponent(entity1, Transform{1.0f, 2.0f, 3.0f});
    ecs.addComponent(entity1, Velocity{0.1f, 0.2f, 0.3f});
    ecs.addComponent(entity2, Transform{4.0f, 5.0f, 6.0f});

    Transform* tmp = ecs.getComponent<Transform>(entity1);
    if (tmp) tmp->x = 2; 
    // Retrieve components
    Transform* t1 = ecs.getComponent<Transform>(entity1);
    if (t1) std::cout << "Entity 1 Transform: " << t1->x << ", " << t1->y << ", " << t1->z << "\n";
    Velocity* v1 = ecs.getComponent<Velocity>(entity1);
    if (v1) std::cout << "Entity 1 Velocity: " << v1->dx << ", " << v1->dy << ", " << v1->dz << "\n";
    Transform* t2 = ecs.getComponent<Transform>(entity2);
    if (t2) std::cout << "Entity 2 Transform: " << t2->x << ", " << t2->y << ", " << t2->z << "\n";

    constexpr size_t entityCount = 100000;

    std::cout << "Timing " << entityCount << " components.\n";

    { // Entity creation benchmark
        Timer timer("Entity Creation");
        for (size_t i = 0; i < entityCount; ++i) {
            ecs.newEntity();
        }
    }

    { // Component addition benchmark
        Timer timer("Component Addition");
        for (size_t i = 0; i < entityCount; ++i) {
            ecs.addComponent(i, Position{static_cast<float>(i), static_cast<float>(i * 2)});
        }
    }

    { // Component retrieval benchmark
        Timer timer("Component Retrieval");
        size_t foundCount = 0;
        for (size_t i = 0; i < entityCount; ++i) {
            bool hasComponent = false;
            Position* pos = ecs.getComponent<Position>(i);
            if (pos) foundCount++;
        }
    }
    printf("----- ENDED BENCHMARK -------\n\n");
}

