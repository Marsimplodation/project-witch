#ifndef SOUL_SHARD_DEFINES_H
#define SOUL_SHARD_DEFINES_H

#include <cstdint>
#define u32 uint32_t
#define SHADOW_CASCADES 4
#define MAX_TEXTURES 1024
#define MAX_MATERIALS 1024
const int MAX_FRAMES_IN_FLIGHT = 2;
const u32 SHADOW_MAP_RES[SHADOW_CASCADES] = { 4096,4096,4096,4096 };  // Different resolutions
const int MAX_INSTANCES = 10000;
const int MAX_DRAWS = 10000;
#endif
