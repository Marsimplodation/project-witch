#include "../SoulShard.h"
#include "glm/fwd.hpp"
#include "tiny_obj_loader.h"
#include <string>
void SoulShard::loadGeometry(std::string modelPath) {    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> obj_materials;
    std::string warn, err;
    

    std::string base_dir = modelPath.substr(0, modelPath.find_last_of('/'));

    if (!tinyobj::LoadObj(&attrib, &shapes, &obj_materials, &warn, &err, modelPath.c_str(), base_dir.c_str())) {
        throw std::runtime_error(warn + err);
    }
    auto & vertices = gpuGeometry.vertices;
    auto & indices = gpuGeometry.indices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
   
    u32 count = 0;
    for (const auto& shape : shapes) {
        u32 faceIndex = 0;
        u32 startIdx = indices.size();
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.position = glm::vec4{
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
                1.0f,
            };
            // Retrieve material index
            vertex.color = glm::vec3(1.0);
            int material_id = shape.mesh.material_ids[faceIndex/3];
            /*vertex.materialIdx = material_id;

            // Add normals
            vertex.normal = glm::vec4{
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2],
                1.0f
            };

            vertex.uv = glm::vec2{
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
            };*/
            /*vertices.push_back(vertex);
            indices.push_back(indices.size());*/

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
            faceIndex++;
            count++;
        }
        u32 endIdx = indices.size();
        Model m{.active = true,
            .indexOffset = startIdx,
            .triangleCount = (endIdx-startIdx)/3,
            .instanceCount = 1,
            .modelMatrices = {glm::mat4(1.0f)},
            .name = shape.name,
        };
        auto entity = entities.create();
        entities.emplace<Model>(entity, m);
    }
}
