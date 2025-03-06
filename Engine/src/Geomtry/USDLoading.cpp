#include "SoulShard.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/base/vt/array.h>
#include <iostream>
#include <vector>

#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/shader.h>
#include <pxr/usd/usdShade/input.h>

// Function to get the transformation matrix of a mesh

pxr::GfMatrix4d GetWorldTransform(pxr::UsdPrim prim) {
    if (!prim) return pxr::GfMatrix4d(1.0);

    pxr::GfMatrix4d worldTransform(1.0);
    for (pxr::UsdPrim p = prim; p; p = p.GetParent()) {
        pxr::UsdGeomXformable xformable(p);
        if (!xformable) continue;

        pxr::GfMatrix4d localTransform;
        bool resetXformStack = false;
        xformable.GetLocalTransformation(&localTransform, &resetXformStack);

        // Multiply transforms from child to parent
        worldTransform = localTransform * worldTransform;

        // If resetXformStack is true, stop accumulating transforms
        if (resetXformStack) break;
    }

    return worldTransform;
}


// Convert pxr::GfMatrix4d to glm::mat4
glm::mat4 ConvertUsdMatrixToGlm(const pxr::GfMatrix4d& usdMatrix) {
    glm::mat4 glmMatrix(1.0f); // Identity matrix

    // Copy USD matrix data into glm::mat4 (column-major order)
    const double* usdData = usdMatrix.GetArray();
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            glmMatrix[col][row] = static_cast<float>(usdData[row * 4 + col]); // Transpose needed
        }
    }

    return glmMatrix;
}

void SoulShard::loadScene(const std::string& file) {
    // Open an existing USD file
    auto & vertices = gpuGeometry.vertices;
    auto & indices = gpuGeometry.indices;
    auto & materials = renderer.data.materials;
    
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    auto stage = pxr::UsdStage::Open(file);
    if (!stage) {
        std::cerr << "Failed to open USD file: " << file << std::endl;
        return;
    }
    printf("loaded %s\n", file.c_str());

    // Traverse the scene and find mesh objects
    (renderer.loadTexture("/home/marius/Downloads/normal.jpg"));
    for (pxr::UsdPrim prim : stage->Traverse()) {
        if (!prim.IsA<pxr::UsdGeomMesh>()) continue;
        u32 faceIndex = 0;
        u32 startIdx = indices.size();
        pxr::UsdGeomMesh mesh(prim);

        // Get vertex positions
        pxr::VtArray<pxr::GfVec3f> points;
        pxr::VtArray<pxr::GfVec3f> normals;
        pxr::VtArray<pxr::GfVec2f> uvs;
        mesh.GetPointsAttr().Get(&points);
        mesh.GetNormalsAttr().Get(&normals);
        pxr::UsdAttribute uvPrimvar = mesh.GetPrim().GetAttribute(pxr::TfToken("primvars:st")); // "st" is common for UVs
        if (uvPrimvar) {
            uvPrimvar.Get(&uvs);
        }
        
        u32 vIdx = 0;
        for (const auto& p : points) {
            auto & n = normals[vIdx];
            auto & uv = uvs[vIdx];
            Vertex vertex {
                .position = glm::vec3(p[0], p[1], p[2]),
                .normal = glm::vec3(n[0], n[1], n[2]),
                .uv = {uv[0], uv[1]},
                .materialIdx = (u32)0,
            };
            //vertices.push_back(vertex);
            vIdx++;
        }
        // Get face indices
        pxr::VtArray<int> faceVertexIndices;
        pxr::VtArray<int> faceNormalIndicies;
        pxr::VtArray<int> faceVertexCounts;
        mesh.GetFaceVertexIndicesAttr().Get(&faceVertexIndices);
        mesh.GetFaceVertexCountsAttr().Get(&faceVertexCounts);

        // Get the material binding API for this mesh
        
        std::unordered_map<int, pxr::SdfPath> faceToMaterialMap;

        // Get all face subsets (each subset represents a material assignment)
        std::vector<pxr::UsdGeomSubset> subsets = pxr::UsdGeomSubset::GetAllGeomSubsets(mesh);

        for (const auto& subset : subsets) {
            // Get the material bound to this subset (face group)
            pxr::UsdShadeMaterialBindingAPI subsetBinding(subset);
            pxr::UsdRelationship subsetMaterialRel = subsetBinding.GetDirectBindingRel();

            if (subsetMaterialRel) {
                pxr::SdfPathVector materialPaths;
                subsetMaterialRel.GetTargets(&materialPaths);

                if (!materialPaths.empty()) {
                    pxr::SdfPath materialPath = materialPaths[0];

                    // Get the face indices associated with this material
                    pxr::VtArray<int> faceIndices;
                    subset.GetIndicesAttr().Get(&faceIndices);

                    for (int faceIdx : faceIndices) {
                        faceToMaterialMap[faceIdx] = materialPath;
                    }
                }
            }
        }
        u32 dataIdx = 0;
        u32 materialIdx = 0;
        for (int index : faceVertexIndices) {
            if(index%3 == 0)faceIndex++;
                auto & p = points[index];
                auto & n = normals[dataIdx];
                auto & uv = uvs[dataIdx++];
                Vertex vertex {
                    .position = glm::vec3(p[0], p[1], p[2]),
                    .normal = glm::vec3(n[0], n[1], n[2]),
                    .uv = {uv[0], 1-uv[1]},
                    .materialIdx = (u32)-1,
                };
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
        u32 endIdx = indices.size();
        GeometryInfo m{
            //.aabb = {.min = min, .max = max},
            .active = true,
            .indexOffset = startIdx,
            .triangleCount = (endIdx-startIdx)/3,
        };
        scene.geometry[prim.GetName()] = m;
        auto & instance = scene.instantiateModel(prim.GetName(), prim.GetName());
        scene.registry.get<TransformComponent>(instance.entity).mat = ConvertUsdMatrixToGlm( GetWorldTransform(mesh.GetPrim()));
    }
}

