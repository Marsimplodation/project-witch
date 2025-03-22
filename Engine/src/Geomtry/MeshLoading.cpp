#include "../SoulShard.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/types.h"
#include "glm/fwd.hpp"
#include "types/types.h"
#include <cmath>
#include <string>
#include <unordered_map>
void SoulShard::loadGeometry(std::string modelPath) {    
    Assimp::Importer importer;
    const aiScene * scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("Assimp error: " + std::string(importer.GetErrorString()));
    }
    auto & vertices = gpuGeometry.vertices;
    auto & indices = gpuGeometry.indices;
    auto & materials = renderer.data.materials;
    std::string base_dir = modelPath.substr(0, modelPath.find_last_of('/'));
    std::unordered_map<std::string, u32> textureAtlas;
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    u32 materialIdx = renderer.data.usedMaterials;
    u32 materialOffset = renderer.data.usedMaterials;
    const int DIFFUSE = 0;
    const int NORMAL = 1;
    for (u32 i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];
        auto & rMat = materials[materialIdx++];

    aiString texturePath;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
            bool isAbsolute = texturePath.C_Str()[0] == '/';
            std::string texture = isAbsolute? texturePath.C_Str() : base_dir + "/" + texturePath.C_Str();
            if (textureAtlas.find(texture) == textureAtlas.end()) {
                u32 id = renderer.loadTexture(texture);
                textureAtlas[texture] = id;
                rMat.texInfos[0] = id;
            } else rMat.texInfos[0] = textureAtlas[texture];
        } else rMat.texInfos[0] = (u32)-1;

        if (material->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == AI_SUCCESS) {
            bool isAbsolute = texturePath.C_Str()[0] == '/';
            std::string texture = isAbsolute? texturePath.C_Str() : base_dir + "/" + texturePath.C_Str();
            if (textureAtlas.find(texture) == textureAtlas.end()) {
                u32 id = renderer.loadTexture(texture);
                textureAtlas[texture] = id;
                rMat.texInfos[1] = id;
            } else rMat.texInfos[1] = textureAtlas[texture];
        } else rMat.texInfos[1] = (u32)-1;

        aiColor3D color(1.0f, 1.0f, 1.0f);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        rMat.albedo = glm::vec4(color.r, color.g, color.b, 1.0f);
    }
    renderer.data.usedMaterials = materialIdx;
   
    u32 count = 0;
    std::vector<Vertex> tmpVertices;
    for (u32 i = 0; i < scene->mNumMeshes; i++) {
        tmpVertices.clear();
        auto & vertices = gpuGeometry.vertices;
        auto & indices = gpuGeometry.indices;
        u32 startIdx = indices.size();

        aiMesh* mesh = scene->mMeshes[i];
        glm::vec3 min = glm::vec3(INFINITY);
        glm::vec3 max = glm::vec3(-INFINITY);
        u32 faceIndex = 0;
        for (u32 j = 0; j < mesh->mNumVertices; j++) {
            Vertex vertex{};
            vertex.position = glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
            vertex.normal = mesh->HasNormals() ? glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z) : glm::vec3(0);
            vertex.uv = mesh->HasTextureCoords(0) ? glm::vec2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y) : glm::vec2(0);
            vertex.materialIdx = mesh->mMaterialIndex + materialOffset;
            
            min = glm::min(min, vertex.position);
            max = glm::max(max, vertex.position);

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            tmpVertices.push_back(vertex);
        }
        for (u32 j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            for (u32 k = 0; k < face.mNumIndices; k++) {
                indices.push_back(uniqueVertices[tmpVertices[face.mIndices[k]]]);
            }
        }
        u32 endIdx = indices.size();
        GeometryInfo m{
            .aabb = {.min = min, .max = max},
            .active = true,
            .indexOffset = startIdx,
            .triangleCount = (endIdx-startIdx)/3,
        };
        auto name = mesh->mName.C_Str(); 
        this->scene.geometry[name] = this->scene.geometryList.size();
        this->scene.geometryList.push_back(m);
        auto & instance = this->scene.instantiateModel(name, name);
    }
}
