#ifndef SOUL_SHARD_TYPES_H
#define SOUL_SHARD_TYPES_H
#include <cstdint>
#include <array>
#include <glm/fwd.hpp>
#include <vector>

//glm imports
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan_core.h>

#define u32 uint32_t
struct Model{
    u32 indexOffset;
    u32 triangleCount;
    u32 instanceCount;
    std::vector<glm::mat4> modelMatrices;
};

struct Camera {
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 rotation; //roll pitch yaw
};

struct Vertex{
    glm::vec4 position;
    glm::vec3 color;
    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

#endif // !SOULS_SHARD_TYPES_H


