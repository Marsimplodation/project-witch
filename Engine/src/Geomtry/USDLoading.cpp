#include "SoulShard.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/vt/array.h>
#include <iostream>
#include <vector>

void SoulShard::loadScene(const std::string& file) {
    // Open an existing USD file
    auto & vertices = gpuGeometry.vertices;
    auto & indices = gpuGeometry.indices;
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
        std::cout << "Found Mesh: " << prim.GetPath() << std::endl;

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
        printf("%zu\n", uvs.size());
        printf("%zu\n", points.size());
        printf("%zu\n", normals.size());
        
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
        u32 dataIdx = 0;
        for (int index : faceVertexIndices) {
                auto & p = points[index];
                auto & n = normals[dataIdx];
                auto & uv = uvs[dataIdx++];
                Vertex vertex {
                    .position = glm::vec3(p[0], p[1], p[2]),
                    .normal = glm::vec3(n[0], n[1], n[2]),
                    .uv = {uv[0], 1-uv[1]},
                    .materialIdx = (u32)0,
                };
            vertices.push_back(vertex);
            indices.push_back(indices.size());
        }
        printf("%u\n", dataIdx);
        u32 endIdx = indices.size();
        GeometryInfo m{
            //.aabb = {.min = min, .max = max},
            .active = true,
            .indexOffset = startIdx,
            .triangleCount = (endIdx-startIdx)/3,
        };
        scene.geometry[prim.GetName()] = m;
        auto & instance = scene.instantiateModel(prim.GetName(), prim.GetName());
    }
}

