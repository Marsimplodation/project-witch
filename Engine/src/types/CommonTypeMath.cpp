#include "types.h"
glm::mat3 createRotationMatrix(float yaw, float pitch, float roll) {
    // Convert degrees to radians
    float cy = cos(glm::radians(yaw));   // cos(yaw)
    float sy = sin(glm::radians(yaw));   // sin(yaw)
    float cp = cos(glm::radians(pitch)); // cos(pitch)
    float sp = sin(glm::radians(pitch)); // sin(pitch)
    float cr = cos(glm::radians(roll));  // cos(roll)
    float sr = sin(glm::radians(roll));  // sin(roll)

    // Compute the rotation matrix in the order of roll (R), pitch (P), yaw (Y)
    glm::mat3 dest=glm::mat3();
    dest[0][0] = cy * cr;
    dest[0][1] = cy * sr;
    dest[0][2] = -sy;

    dest[1][0] = sp * sy * cr - cp * sr;
    dest[1][1] = sp * sy * sr + cp * cr;
    dest[1][2] = sp * cy;

    dest[2][0] = cp * sy * cr + sp * sr;
    dest[2][1] = cp * sy * sr - sp * cr;
    dest[2][2] = cp * cy;
    return dest;
}

