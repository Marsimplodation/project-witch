#ifndef LIGHT_H
#define LIGHT_H
const mat4 bias = mat4( 
  0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.5, 0.5, 0.0, 1.0 );
#define SHADOW_CASCADES 4
struct DirectionLight {
    vec4 position; 
    vec4 direction; 
    mat4 views[SHADOW_CASCADES];
    mat4 projections[SHADOW_CASCADES];
    vec4 color;
    float intensity; 
    float splitDepths[SHADOW_CASCADES];
};
#endif
