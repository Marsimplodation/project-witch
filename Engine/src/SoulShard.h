#ifndef SOUL_SHARD_ENINGE_H
#define SOUL_SHARD_ENINGE_H
#include "entt/entt.hpp"
#include "types/types.h"
#include <vector>
struct SoulShard {
	std::vector<void(*)(float deltaTime)> systems;
	entt::registry entities;
	std::vector<Vertex> vertices;
	std::vector<u32> indices;
	int run();
};

#endif // !SOUL_SHARD_ENINGE_Hf
