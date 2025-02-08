#ifndef LIGHT_H
#define LIGHT_H
struct DirectionLight {
    mat4 view;
    mat4 projection;
    vec4 color;
    float intensity; 
};
#endif
