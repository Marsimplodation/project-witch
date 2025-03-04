#ifndef SOUL_SHARD_DEFINES_H
#define SOUL_SHARD_DEFINES_H

#include <cstdint>
#define u32 uint32_t
#define SHADOW_CASCADES 4
const int MAX_FRAMES_IN_FLIGHT = 2;
const u32 SHADOW_MAP_RES[SHADOW_CASCADES] = { 4096,4096,4096,4096 };  // Different resolutions
#endif
