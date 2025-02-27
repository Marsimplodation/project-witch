#include "../SoulShard.h"
#include "glm/fwd.hpp"
#include "tiny_obj_loader.h"
#include <cmath>
#include <string>
#include <unordered_map>
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
    std::unordered_map<std::string, u32> textureAtlas;
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    std::vector<u32> tex_materials(0);

    for (const auto & material : obj_materials) {
        if(!material.diffuse_texname.empty()) {
            bool isAbsolute = material.diffuse_texname.front() == '/';
            std::string texture = isAbsolute? material.diffuse_texname : base_dir + "/" + material.diffuse_texname;
            if (textureAtlas.find(texture) == textureAtlas.end()) {
                u32 id = renderer.loadTexture(texture); 
               textureAtlas[texture] = id;
                tex_materials.push_back(id);
            } else tex_materials.push_back(textureAtlas[texture]); 
        } else tex_materials.push_back(-1); 
    }
   
    u32 count = 0;
    for (const auto& shape : shapes) {
        glm::vec3 min = glm::vec3(INFINITY);
        glm::vec3 max = glm::vec3(-INFINITY);
        u32 faceIndex = 0;
        u32 startIdx = indices.size();
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.position = glm::vec3{
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
            };
            min[0] = glm::min(vertex.position[0], min[0]);
            min[1] = glm::min(vertex.position[1], min[1]);
            min[2] = glm::min(vertex.position[2], min[2]);
            max[0] = glm::max(vertex.position[0], max[0]);
            max[1] = glm::max(vertex.position[1], max[1]);
            max[2] = glm::max(vertex.position[2], max[2]);
            // Retrieve material index
            int material_id = tex_materials[shape.mesh.material_ids[faceIndex/3]];
            vertex.materialIdx = material_id;

            // Add normals
            vertex.normal = glm::vec3{
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };

            vertex.uv = glm::vec2{
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
            };
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
        GeometryInfo m{
            .aabb = {.min = min, .max = max},
            .active = true,
            .indexOffset = startIdx,
            .triangleCount = (endIdx-startIdx)/3,
        };
        scene.geometry[shape.name] = m;
        auto & instance = scene.instantiateModel(shape.name, shape.name);
    }
}
