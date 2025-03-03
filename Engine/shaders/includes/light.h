#ifndef LIGHT_H
#define LIGHT_H
const mat4 bias = mat4( 
  0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.5, 0.5, 0.0, 1.0 );
#define SHADOW_CASCADES 4
const uint SHADOW_MAP_RES[SHADOW_CASCADES] =  { 4096,4096,4096,4096 }; // Different resolutions

struct DirectionLight {
    mat4 views[SHADOW_CASCADES];  // 16-byte alignment
    mat4 projections[SHADOW_CASCADES]; // 16-byte alignment
    vec4 position;               // 16-byte alignment
    vec4 direction;              // 16-byte alignment
    vec4 color;                  // 16-byte alignment
    vec4 splitDepths; // this can follow vectors without padding
    bool castShadows;
};

#endif
