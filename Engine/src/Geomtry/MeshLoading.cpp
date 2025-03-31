#include "../SoulShard.h"
#include "Scene/Scene.h"
#include "assimp/Importer.hpp"
#include "assimp/light.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/types.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "types/types.h"
#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

glm::mat4 ConvertMatrixToGLM(const aiMatrix4x4& from) {
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

glm::mat4 GetMeshTransformGLM(const aiScene* scene, aiMesh* mesh) {
    aiNode* node = nullptr;

    // Find the node that contains this mesh
    for (u32 i = 0; i < scene->mRootNode->mNumChildren; i++) {
        aiNode* child = scene->mRootNode->mChildren[i];
        for (u32 j = 0; j < child->mNumMeshes; j++) {
            if (scene->mMeshes[child->mMeshes[j]] == mesh) {
                node = child;
                break;
            }
        }
        if (node) break;
    }

    // Accumulate transformations up the hierarchy
    aiMatrix4x4 transform;
    if (node) {
        transform = node->mTransformation;
        aiNode* parent = node->mParent;
        while (parent) {
            transform = parent->mTransformation * transform;
            parent = parent->mParent;
        }
    }

    return ConvertMatrixToGLM(transform);
}

glm::mat4 GetLightTransformGLM(const aiScene* scene, aiLight* light) {
    aiNode* node = scene->mRootNode->FindNode(light->mName);
    if (!node) return glm::mat4(1.0f);  

    // Accumulate transformations up the hierarchy
    aiMatrix4x4 transform = node->mTransformation;
    aiNode* parent = node->mParent;
    while (parent) {
        transform = parent->mTransformation * transform;
        parent = parent->mParent;
    }

    return ConvertMatrixToGLM(transform);
}


void loadMaterials(std::vector<Material> & materials,
                   u32 & materialIdx,
                   const aiScene * scene,
                   VkRenderer & renderer,
                   std::string & base_dir) {
    const int DIFFUSE = 0;
    const int NORMAL = 1;
    std::unordered_map<std::string, u32> textureAtlas;
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

}


void loadMeshes(u32 & materialOffset,
                   const aiScene * fileScene,
                   VkRenderer & renderer,
                    std::vector<Vertex> & vertices,
                    std::vector<u32> & indices,
                   std::string & base_dir,
                Scene & scene,
                bool objFile) {
    u32 count = 0;
    std::vector<Vertex> tmpVertices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (u32 i = 0; i < fileScene->mNumMeshes; i++) {
        tmpVertices.clear();
        u32 startIdx = indices.size();

        aiMesh* mesh = fileScene->mMeshes[i];
        u32 faceIndex = 0;
        auto min = glm::vec3(MAXFLOAT);
        auto max = glm::vec3(-MAXFLOAT);
        for (u32 j = 0; j < mesh->mNumVertices; j++) {
            Vertex vertex{};
            vertex.position = glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
            min = glm::min(min, vertex.position);
            max = glm::max(max, vertex.position);
        }
        //estimate mesh position for obj files which only ship absolute triangle positions
        glm::vec3 center = (max + min) * 0.5f;
        if(objFile) {
            max-=center;
            min-=center;
        }


        for (u32 j = 0; j < mesh->mNumVertices; j++) {
            Vertex vertex{};
            vertex.position = glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
            if(objFile) vertex.position -= center;
            vertex.normal = mesh->HasNormals() ? glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z) : glm::vec3(0);
            vertex.uv = mesh->HasTextureCoords(0) ? glm::vec2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y) : glm::vec2(0);
            vertex.materialIdx = mesh->mMaterialIndex + materialOffset;
            

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
        scene.geometry[name] = scene.geometryList.size();
        scene.geometryList.push_back(m);
        auto & instance = scene.instantiateModel(name, name);
        auto tPtr = ECS::getComponent<TransformComponent>(instance.entity);
        if(!tPtr) continue; 
        if(objFile) tPtr->mat = glm::translate(glm::mat4(1.0f), center);
        else tPtr->mat = GetMeshTransformGLM(fileScene, mesh);
    }}

void loadLights(const aiScene * fileScene,
                Scene & scene) {
    if (!fileScene || !fileScene->HasLights()) {
        return;
    }

    for (unsigned int i = 0; i < fileScene->mNumLights; ++i) {
        aiLight* light = fileScene->mLights[i];

        if (light->mType == aiLightSource_POINT) {
            aiVector3D pos = light->mPosition;
            aiColor3D color = light->mColorDiffuse;  // Diffuse color
            scene.createPointLight();
            auto instance = scene.instances.back();
            auto * pointLight = ECS::getComponent<PointLight>(instance.entity);
            auto * name = ECS::getComponent<InstanceName>(instance.entity);
            auto * trans = ECS::getComponent<TransformComponent>(instance.entity);
            if(!name || !pointLight || !trans) continue;
            glm::vec3 c = {color.r, color.g, color.b};
            float intensity = length(c);
            intensity /= 100.0f;

            // Normalize color
            float maxChannel = std::max({color.r, color.g, color.b});
            glm::vec3 normalizedColor = (maxChannel > 0.0f) ? glm::vec3(color.r, color.g, color.b) / maxChannel : glm::vec3(1.0f);

            // Assign final color with intensity
            pointLight->color = { normalizedColor.r, normalizedColor.g, normalizedColor.b, intensity };
            trans->mat = GetLightTransformGLM(fileScene, light);
        }
        if (light->mType == aiLightSource_DIRECTIONAL) {
            aiVector3D direction = light->mDirection;
            aiColor3D color = light->mColorDiffuse;  // Diffuse color
            glm::vec3 c = {color.r, color.g, color.b};
            float intensity = length(c);
            intensity /= 100.0f;
            // Normalize color
            float maxChannel = std::max({color.r, color.g, color.b});
            glm::vec3 normalizedColor = (maxChannel > 0.0f) ? glm::vec3(color.r, color.g, color.b) / maxChannel : glm::vec3(1.0f);

            // Assign final color with intensity
            scene.sceneLight.color ={ normalizedColor.r, normalizedColor.g, normalizedColor.b, intensity };
            auto pos = GetLightTransformGLM(fileScene, light)[3];
            pos = glm::normalize(pos);
            scene.sceneLight.direction = {
                -pos.x,
                -pos.y,
                -pos.z,
                1.0f,
            }; 
        }
    }
}

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
    u32 materialIdx = renderer.data.usedMaterials;
    u32 materialOffset = renderer.data.usedMaterials;
    renderer.data.usedMaterials = materialIdx;
   
    bool objFile =  modelPath.find(".obj") != std::string::npos;
    loadMaterials(materials, materialIdx, scene, renderer, base_dir);
    loadMeshes(materialOffset, scene, renderer, vertices, indices, base_dir,this->scene, objFile);
    loadLights(scene, this->scene);
    
}
