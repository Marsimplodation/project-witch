#ifndef SOUL_SHARD_TYPES_H
#define SOUL_SHARD_TYPES_H
#include <Jolt/Jolt.h>
#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include <cstdint>
#include <array>
#include <glm/fwd.hpp>
#include <string>
#include <vector>

//glm imports
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <vulkan/vulkan_core.h>

#define u32 uint32_t
struct Model{
    u32 indexOffset;
    u32 triangleCount;
    u32 instanceCount;
};

struct TransformComponent{
    glm::mat4 mat;
    glm::vec3 rot;
    glm::vec3 pos;
    glm::vec3 scale;
};

struct DirectionLight {
    glm::vec4 position; 
    glm::vec4 direction; 
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 color;
    float intensity; 
};

struct PhysicsComponent {
    JPH::BodyID bodyID;
    JPH::Body* body;
    JPH::BoxShapeSettings* shapeSettings;
};

struct System{
    bool active;
    void(*func)(float deltaTime);
    std::string name;
};

struct Camera {
    glm::mat4 view;
    glm::mat4 projection;
};

struct Vertex{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    u32 materialIdx;
    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
    bool operator==(const Vertex& other) const;
};
template<> struct std::hash<Vertex> {
    size_t operator()(Vertex const& vertex) const;
};struct GPUGeometry {
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
};

//print overloads
void print(glm::vec4 & v);
void print(glm::vec3 & v);

//common math
glm::mat3 createRotationMatrix(float yaw, float pitch, float roll);
#endif // !SOULS_SHARD_TYPES_H


