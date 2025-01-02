#include "types.h"
void print(glm::vec4 & v){
    printf("x: %f, y: %f, z: %f, w: %f\n", v.x, v.y, v.z, v.w);
}
void print(glm::vec3 & v){
    printf("x: %f, y: %f, z: %f\n", v.x, v.y, v.z);
}
