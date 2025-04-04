#include "types.h"
#include <vulkan/vulkan_core.h>
VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
     return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> Vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, uv);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32_UINT;
    attributeDescriptions[3].offset = offsetof(Vertex, materialIdx);
    return attributeDescriptions;
}


bool Vertex::operator==(const Vertex& other) const {
    return position == other.position 
            && materialIdx == other.materialIdx
            && uv == other.uv;
}
namespace std {
    inline void hash_combine(std::size_t& seed, std::size_t hash) {
        // A simple and effective hash combination function
        seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    template <> struct hash<glm::vec4> {
            std::size_t operator()(const glm::vec4& v) const {
                std::size_t seed = 0;
                std::size_t h1 = std::hash<float>()(v.x);
                std::size_t h2 = std::hash<float>()(v.y);
                std::size_t h3 = std::hash<float>()(v.z);
                std::size_t h4 = std::hash<float>()(v.w);
                // Combine the hashes of x, y, z, and w
                // Combine the hashes
                hash_combine(seed, h1);
                hash_combine(seed, h2);
                hash_combine(seed, h3);
                hash_combine(seed, h4);

                return seed;
            }
        };
    template <> struct hash<glm::vec2> {
            std::size_t operator()(const glm::vec2& v) const {
                std::size_t seed = 0;
                std::size_t h1 = std::hash<float>()(v.x);
                std::size_t h2 = std::hash<float>()(v.y);
                // Combine the hashes of x, y, z, and w
                // Combine the hashes
                hash_combine(seed, h1);
                hash_combine(seed, h2);

                return seed;
            }
    };

    template <> struct hash<glm::vec3> {
            std::size_t operator()(const glm::vec3& v) const {
                std::size_t seed = 0;
                std::size_t h1 = std::hash<float>()(v.x);
                std::size_t h2 = std::hash<float>()(v.y);
                std::size_t h3 = std::hash<float>()(v.z);
                // Combine the hashes of x, y, z, and w
                // Combine the hashes
                hash_combine(seed, h1);
                hash_combine(seed, h2);
                hash_combine(seed, h3);

                return seed;
            }
    };
    size_t hash<Vertex>::operator()(Vertex const& vertex) const {
        std::size_t seed = 0;

        // Hash individual fields
        std::size_t h1 = std::hash<glm::vec3>()(vertex.position);
        std::size_t h2 = std::hash<glm::vec3>()(vertex.normal);
        std::size_t h3 = std::hash<glm::vec2>()(vertex.uv);
        std::size_t h4 = std::hash<u32>()(vertex.materialIdx);

        // Combine the hashes
        hash_combine(seed, h1);
        hash_combine(seed, h2);
        hash_combine(seed, h3);
        hash_combine(seed, h4);

        return seed;
    }
}
