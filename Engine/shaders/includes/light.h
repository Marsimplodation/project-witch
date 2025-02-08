#ifndef LIGHT_H
#define LIGHT_H
const mat4 bias = mat4( 
  0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.5, 0.5, 0.0, 1.0 );
struct DirectionLight {
    vec4 position; 
    vec4 direction; 
    mat4 view;
    mat4 projection;
    vec4 color;
    float intensity; 
};
#endif
