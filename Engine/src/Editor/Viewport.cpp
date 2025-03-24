#include "Editor.h"
#include "../SoulShard.h"
#include "../Vulkan/VkRenderer.h"
#include "ImGuizmo.h"
#include "InputHandling/KeyDefines.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "types/ECS.h"
#include <cfloat>
namespace {
    float fov = 70;
    glm::vec3 position = glm::vec3(0,0,15.0f);
    glm::vec3 forward = glm::vec3(0,0,-1.0f);
    glm::vec3 up = glm::vec3(0,1,0.0f);
    glm::vec3 right = glm::vec3(1,0,0.0f);

    float yaw = 0.0f;
    float pitch = 0.0f;
}


OptionalEntityID getInstance(glm::vec3 origin, glm::vec3 dir, Scene & scene) {
    float tmax = FLT_MAX;
    float tmin = 0.0f;
    glm::vec3 inv_dir = 1.0f / dir;
    OptionalEntityID selected;

    auto findAABBIntersection = [&](AABB &b) {
        // calculate intersection intervals
        float tx1 = (b.min.x - origin.x) * inv_dir.x;
        float tx2 = (b.max.x - origin.x) * inv_dir.x;
        float ty1 = (b.min.y - origin.y) * inv_dir.y;
        float ty2 = (b.max.y - origin.y) * inv_dir.y;
        float tz1 = (b.min.z - origin.z) * inv_dir.z;
        float tz2 = (b.max.z - origin.z) * inv_dir.z;
        // find min and max intersection t’s
        using std::max;
        using std::min;
        float tres[2] = {
            (max(max(min(tx1, tx2), min(ty1, ty2)), min(tz1, tz2))),
            min(min(min(max(tx1, tx2), max(ty1, ty2)), max(tz1, tz2)), tmax)
        };
        // return result
        if (tres[0] <= tres[1] && tres[0] >= 0) {
            tmax = tres[0];
            return true;
        }
        return false;
    };
    for(auto & instance : scene.instances) {
        auto aabb = ECS::getComponent<AABB>(instance.entity);
        if(!aabb) continue;
        if(!findAABBIntersection(*aabb)) continue;
        selected = instance.entity;
    }
    return selected;
}

void ImguiModule::renderViewport(void * initPtr, void * dataPtr) {
    VkRenderer::Init & init = * (VkRenderer::Init*)initPtr;
    VkRenderer::RenderData & data = *(VkRenderer::RenderData*) dataPtr;

    ImGui::Begin("Viewport");
    if(ImGui::GetWindowSize().x != previewSize.x || ImGui::GetWindowSize().y != previewSize.y) {
        data.swapchainOutOfDate = true;
    }
    ImGui::Image(sceneViews[data.currentFrame],previewSize);
    previewSize = ImGui::GetWindowSize();
    auto & engine = *(SoulShard*)enginePtr;
    auto & input = engine.inputHandler;
    engine.editorCamera.fov = glm::radians(fov);
    engine.editorCamera.projection = glm::perspective(
                                        engine.editorCamera.fov,
                                        engine.renderingResolution[0] / engine.renderingResolution[1],
                                        0.1f, 1000.0f);
    engine.editorCamera.near=0.1f;
    engine.editorCamera.far=1000.0f;
    engine.editorCamera.projection[1][1] *= -1;
    engine.editorCamera.view = glm::lookAt(position,
                                    position + forward,
                                    up); 


    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
    if(ImGui::IsWindowHovered() && input.isKeyPressed(MOUSE_1) && input.isKeyPressed(KEY_LEFT_CONTROL)) {
        ImVec2 windowPos = ImGui::GetWindowPos();       // Absolute position of the window
        ImVec2 windowPadding = ImGui::GetStyle().WindowPadding;  // Internal padding
        ImVec2 windowSize = ImGui::GetWindowSize() - windowPadding - windowPadding;
        ImVec2 firstElementPos = ImVec2(
            windowPos.x + windowPadding.x,
            windowPos.y + ImGui::GetFrameHeight() + windowPadding.y
        );
        ImVec2 mouse = ImGui::GetMousePos() - firstElementPos;
        auto ndc = mouse / windowSize;
        float ndcX = 2*(ndc.x)-1.0f;
        float ndcY = 2*(ndc.y) - 1.0f;
        glm::vec4 nearPoint = glm::vec4(ndcX, ndcY, -1.0, 1.0);
        glm::vec4 farPoint  = glm::vec4(ndcX, ndcY, 1.0, 1.0);

        glm::mat4 invVP = glm::inverse(engine.editorCamera.projection * engine.editorCamera.view);
        glm::vec4 rayStart = invVP * nearPoint;
        glm::vec4 rayEnd   = invVP * farPoint;

        rayStart /= rayStart.w;
        rayEnd /= rayEnd.w;

        glm::vec3 rayDirection = glm::normalize(glm::vec3(rayEnd - rayStart));

        // Check for intersection with objects
        selectedInstance = getInstance(rayStart, rayDirection, engine.scene);
    }

    if(ImGui::IsWindowHovered() && input.isKeyPressed(MOUSE_2)) {
        float deltaTime = engine.deltaTime; 
        input.captureMouse();
        auto delta = input.getMouseDelta();
        up = glm::vec3(engine.editorCamera.view[1]);
        right = glm::vec3(engine.editorCamera.view[0]);
        forward = glm::vec3(engine.editorCamera.view[2]);
        yaw -=  delta[0] * deltaTime * 50.0f;
        pitch -= delta[1] * deltaTime * 50.0f;
        auto rot = createRotationMatrix(yaw, pitch, 0.0f);
        glm::vec3 f = {0,0,-1};
        glm::vec3 u = {0,1,0};
        glm::vec3 r = {1,0,0};

        forward = rot * f;
        up = rot * u;
        right = rot * r;
        if(input.isKeyPressed(KEY_W)) position += forward * deltaTime * 10.0f;
        if(input.isKeyPressed(KEY_S)) position -= forward * deltaTime * 10.0f;
        if(input.isKeyPressed(KEY_D)) position += right * deltaTime * 10.0f;
        if(input.isKeyPressed(KEY_A)) position -= right * deltaTime * 10.0f;
    } else input.releaseMouse(); 
    ImGui::End();
}
