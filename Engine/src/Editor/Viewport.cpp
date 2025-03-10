#include "Editor.h"
#include "../SoulShard.h"
#include "../Vulkan/VkRenderer.h"
#include "ImGuizmo.h"
#include "InputHandling/KeyDefines.h"
#include "imgui.h"
#include "imgui_internal.h"
namespace {
    float fov = 70;
    glm::vec3 position = glm::vec3(0,0,15.0f);
    glm::vec3 forward = glm::vec3(0,0,-1.0f);
    glm::vec3 up = glm::vec3(0,1,0.0f);
    glm::vec3 right = glm::vec3(1,0,0.0f);

    float yaw = 0.0f;
    float pitch = 0.0f;
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
