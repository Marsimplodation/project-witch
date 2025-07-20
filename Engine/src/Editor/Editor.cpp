#include "Editor.h"
#include "Editor/NodeEditor.h"
#include "GLFW/glfw3.h"
#include <cmath>
#include <iterator>
#include "../Vulkan/VkRenderer.h"
#include "ImGuizmo.h"
#include "InputHandling/KeyDefines.h"
#include "SoulShard.h"
#include "glm/detail/qualifier.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "types/ECS.h"
#include "types/ECS_UI.h"
#include "types/types.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <string>
#include <vulkan/vulkan_core.h>

void customTheme() {
auto &colors = ImGui::GetStyle().Colors;
colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
colors[ImGuiCol_MenuBarBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Border
colors[ImGuiCol_Border] = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
colors[ImGuiCol_BorderShadow] = ImVec4{0.0f, 0.0f, 0.0f, 0.24f};

// Text
colors[ImGuiCol_Text] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};
colors[ImGuiCol_TextDisabled] = ImVec4{0.5f, 0.5f, 0.5f, 1.0f};

// Headers
colors[ImGuiCol_Header] = ImVec4{0.13f, 0.13f, 0.17, 1.0f};
colors[ImGuiCol_HeaderHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
colors[ImGuiCol_HeaderActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Buttons
colors[ImGuiCol_Button] = ImVec4{0.13f, 0.13f, 0.17, 1.0f};
colors[ImGuiCol_ButtonHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
colors[ImGuiCol_ButtonActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
colors[ImGuiCol_CheckMark] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};

// Popups
colors[ImGuiCol_PopupBg] = ImVec4{0.1f, 0.1f, 0.13f, 0.92f};

// Slider
colors[ImGuiCol_SliderGrab] = ImVec4{0.44f, 0.37f, 0.61f, 0.54f};
colors[ImGuiCol_SliderGrabActive] = ImVec4{0.74f, 0.58f, 0.98f, 0.54f};

// Frame BG
colors[ImGuiCol_FrameBg] = ImVec4{0.13f, 0.13, 0.17, 1.0f};
colors[ImGuiCol_FrameBgHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
colors[ImGuiCol_FrameBgActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Tabs
colors[ImGuiCol_Tab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
colors[ImGuiCol_TabHovered] = ImVec4{0.24, 0.24f, 0.32f, 1.0f};
colors[ImGuiCol_TabActive] = ImVec4{0.2f, 0.22f, 0.27f, 1.0f};
colors[ImGuiCol_TabUnfocused] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Title
colors[ImGuiCol_TitleBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
colors[ImGuiCol_TitleBgActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Scrollbar
colors[ImGuiCol_ScrollbarBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
colors[ImGuiCol_ScrollbarGrab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{0.24f, 0.24f, 0.32f, 1.0f};

// Seperator
colors[ImGuiCol_Separator] = ImVec4{0.44f, 0.37f, 0.61f, 1.0f};
colors[ImGuiCol_SeparatorHovered] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};
colors[ImGuiCol_SeparatorActive] = ImVec4{0.84f, 0.58f, 1.0f, 1.0f};

// Resize Grip
colors[ImGuiCol_ResizeGrip] = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
colors[ImGuiCol_ResizeGripHovered] = ImVec4{0.74f, 0.58f, 0.98f, 0.29f};
colors[ImGuiCol_ResizeGripActive] = ImVec4{0.84f, 0.58f, 1.0f, 0.29f};

// Docking
colors[ImGuiCol_DockingPreview] = ImVec4{0.44f, 0.37f, 0.61f, 1.0f};

auto &style = ImGui::GetStyle();
style.TabRounding = 4;
style.ScrollbarRounding = 9;
style.WindowRounding = 7;
style.GrabRounding = 3;
style.FrameRounding = 3;
style.PopupRounding = 4;
style.ChildRounding = 4;
  // Go through every colour and convert it to linear
  // This is because ImGui uses linear colours but we are using sRGB
  // This is a simple approximation of the conversion
  for (int i = 0; i < ImGuiCol_COUNT; i++) {
    /*float linear = (srgb <= 0.04045f) ? srgb / 12.92f : pow((srgb + 0.055f)
     * / 1.055f, 2.4f);*/

    ImVec4 &col = style.Colors[i];
    col.x = col.x <= 0.04045f ? col.x / 12.92f
                              : pow((col.x + 0.055f) / 1.055f, 2.4f);
    col.y = col.y <= 0.04045f ? col.y / 12.92f
                              : pow((col.y + 0.055f) / 1.055f, 2.4f);
    col.z = col.z <= 0.04045f ? col.z / 12.92f
                              : pow((col.z + 0.055f) / 1.055f, 2.4f);
  }
}

NodeEditor editor{};
void ImguiModule::init(void * initPtr, void * dataPtr) {
    VkDescriptorPoolSize pool_sizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkRenderer::Init & init = * (VkRenderer::Init*)initPtr;
    VkRenderer::RenderData & data = *(VkRenderer::RenderData*)dataPtr;

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * MAX_FRAMES_IN_FLIGHT;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VkResult result = vkCreateDescriptorPool(init.device, &pool_info, nullptr, &imguiPool);
    
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = init.instance;
    init_info.PhysicalDevice = init.physicalDevice;
    init_info.Device = init.device;
    init_info.Queue = data.graphicsQueue;
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = init.swapchain.image_count;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.RenderPass = data.renderPass;
    ImGui_ImplVulkan_Init(&init_info);
    ImGui_ImplGlfw_InitForVulkan(init.window, true);
    customTheme();

        editor.nodes.push_back({
            .id = 0,
            .position = {10 , 20 },
            .size = {100,100},
            .outputs = {Pin{0, {-10,15}}},
        });
        editor.nodes.push_back({
            .id = 1,
            .position = {250, 30},
            .size = {100,100},
            .inputs = {Pin{0, {-10,15}}},
        });
        editor.links.push_back({.fromNode = 0, .fromPin = 0, .toNode = 1, .toPin = 0});
}

void ImguiModule::destroy(VkDevice device) {
    // Cleanup (happens once)
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(device, imguiPool, nullptr);
}

bool textureCreated = false;
// Function to format numbers with dots as thousands separators
std::string formatNumberWithDots(size_t number) {
    std::string numStr = std::to_string(number);
    int insertPosition = static_cast<int>(numStr.length()) - 3;
    while (insertPosition > 0) {
        numStr.insert(insertPosition, ".");
        insertPosition -= 3;
    }
    return numStr;
}
bool lineRenderer = false;

struct RadialSliderState {
    bool isActive = false;
    ImVec2 pos;
    ImVec2 lastDelta{0,0};
} sliderState;

// Function to draw radial light direction selector (perimeter only)
void RadialLightSlider(const char* label, float& outX, float& outY, float& outZ){
    RadialSliderState& state = sliderState;

    ImVec2 center = ImGui::GetCursorScreenPos()+ImVec2{20, 0};
    float radius = 50.0f; // Radius of the radial slider
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 delta(0, 0);

    bool isHovered = ImGui::IsWindowHovered();
    bool isClicked = ImGui::IsMouseDown(0);
    bool isInCircle = glm::distance(
        glm::vec2(center.x, center.y)+ glm::vec2(radius, radius),
        glm::vec2(mousePos.x, mousePos.y)) <= radius;

    if (isHovered && isClicked && isInCircle) {
        state.isActive = true;
    } else if (!isClicked) {
        state.isActive = false;
    }

    if (state.isActive) {
        // Compute direction from center
        delta = ImVec2(mousePos.x - (center.x + radius), mousePos.y - (center.y + radius));
        float dist = sqrtf(delta.x * delta.x + delta.y * delta.y);

        // Clamp to circle bounds
        if (dist > radius) {
            delta.x = (delta.x / dist) * radius;
            delta.y = (delta.y / dist) * radius;
        }

        // Store last valid position
        state.pos = ImVec2(center.x + radius + delta.x, center.y + radius + delta.y);
        state.lastDelta = delta;
    }

    // Convert 2D position to 3D unit sphere (full range)
    float nx = (state.lastDelta.x) / radius;     
    float ny = (state.lastDelta.y) / radius;
    float nz = sqrtf(fmaxf(0.0f, 1.0f - nx * nx - ny * ny)); // Compute Z

    // Map to correct light direction
    outX = -nx;
    outY = -nz;  // Middle = -Y, Up = -Z
    outZ = -ny;
    // Draw the circle and handle
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddCircle(center + ImVec2(radius, radius), radius, IM_COL32(255, 255, 255, 255));
    draw_list->AddCircleFilled(state.pos, 5.0f, IM_COL32(255, 0, 0, 255));
    ImGui::Dummy(ImVec2(radius * 2, radius * 2)); // Reserve space
}

std::vector<ImTextureID> texturesPerFrame[MAX_FRAMES_IN_FLIGHT];

void ImguiModule::renderInstance(){
    SoulShard & engine = *((SoulShard*)enginePtr);
    ImGui::Begin("Selected");
    if(!selectedInstance.hasValue()) {
        ImGui::End();
        return;
    }
    //marking 0 as not selected
    EntityID instance = selectedInstance.getValue();
    auto namePtr = engine.scene.registry.getComponent<InstanceName>(instance);
    if(!namePtr) return;
    char * name = namePtr->name;
    bool close = ImGui::Button("✘");
    ImGui::SameLine();
    ImGui::InputText("Name", name, 255);
    auto transformPtr = engine.scene.registry.getComponent<TransformComponent>(instance);
    float entryHeight = ImGui::GetFrameHeightWithSpacing() * 1.2f;
    if(transformPtr) { 
        glm::mat4 & transform = transformPtr->mat; 
        engine.editorCamera.projection[1][1] *= -1;
        ImGuizmo::Manipulate(glm::value_ptr(engine.editorCamera.view),
                         glm::value_ptr(engine.editorCamera.projection),
                         ImGuizmo::OPERATION::UNIVERSAL,
                         ImGuizmo::WORLD,
                         glm::value_ptr(transform));
        engine.editorCamera.projection[1][1] *= -1;
        ImGui::BeginChild("Transform", ImVec2(0,entryHeight * 4),true);
        ImGui::Text("Transform");
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), matrixTranslation, matrixRotation, matrixScale);
        ImGui::DragFloat3("Translation", matrixTranslation);
        ImGui::DragFloat3("Rotation", matrixRotation);
        ImGui::DragFloat3("Scale", matrixScale);
        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, glm::value_ptr(transform));
        ImGui::EndChild();
    }

    ECS_UI::renderEntityInspector(ECS_UI::CHILD);

    if(close) selectedInstance.removeValue();
    ImGui::End();
}

void ImguiModule::update(void * initPtr, void * dataPtr) {
    VkRenderer::Init & init = * (VkRenderer::Init*)initPtr;
    VkRenderer::RenderData & data = *(VkRenderer::RenderData*) dataPtr;
    SoulShard & engine = *((SoulShard*)enginePtr);
    auto & textures = texturesPerFrame[data.currentFrame];

        if(!active) return;
        if(textureCreated) {
            ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(sceneViews[data.currentFrame]));
            for(auto & texture : textures)
            ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(texture));
        }
        textures.resize(data.textures.size() + SHADOW_CASCADES);
        for(int i = 0; i < data.textures.size(); ++i) {
            textures[i] = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(
            data.textures[i].sampler,
            data.textures[i].view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL // Image layout for sampling
        ));
        }
        for(int i = 0; i < SHADOW_CASCADES; ++i) {
            textures[textures.size() - SHADOW_CASCADES + i] = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(
            data.imageSampler,
            data.shadowImageViews[data.currentImgIndex][i],
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL // Image layout for sampling
            ));
        }
        sceneViews[data.currentFrame] = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(
            data.imageSampler,
            data.offscreenImageViews[data.currentImgIndex],                       // VkSampler
            VK_IMAGE_LAYOUT_GENERAL // Image layout for sampling
        ));
        textureCreated = true;

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame(); // If using GLFW
        ImGui::NewFrame();// Set up docking
        ImGuizmo::BeginFrame();

        ImGui::DockSpaceOverViewport(0);
    
        ECS_UI::renderEntityList((EntityID*)engine.scene.instances.data(), engine.scene.instances.size());
        selectedInstance = ECS_UI::currentEntity;


        // Build your GUI
        renderViewport(initPtr, dataPtr);        
        ImGui::Begin("Debug");
        ImGui::Text("FPS %f", 1.0f/((SoulShard*)enginePtr)->deltaTime);
        ImGui::Text("Vertices: %s", formatNumberWithDots(data.vertices->size()).c_str());
        ImGui::Text("Triangles: %s", formatNumberWithDots(data.indices->size() / 3).c_str());
        ImGui::Text("Drawcalls: %d", data.drawCalls);
        ImGui::Text("Instaces Drawn: %d", data.instancesRendered);
        ImGui::End();
        ImGui::Begin("Systems");
        for(auto & system : engine.systems) {
            const char * name = system.name.c_str();
            ImGui::Text("%s", name);
            ImGui::Checkbox((std::string("Active##") + name).c_str(), &system.active);
        }
        ImGui::End();
        ImGui::Begin("Textures");
        for(int i = 0; i < SHADOW_CASCADES; i+=2) {
            ImGui::Image(textures[textures.size() -SHADOW_CASCADES + i], ImVec2(128, 128));
            ImGui::SameLine();
            ImGui::Image(textures[textures.size() -SHADOW_CASCADES + i +1], ImVec2(128, 128));
        }
        for(int i = 0; i < data.textures.size(); ++i) {
            ImGui::Image(textures[i], ImVec2(256, 256));
        }
        ImGui::End();


        ImGui::Begin("Light");
        glm::vec3 & c = *(glm::vec3*)&engine.scene.sceneLight.direction;
        RadialLightSlider("light dir", c.x, c.y, c.z);
        ImGui::DragFloat3("Direction", (float*)&engine.scene.sceneLight.direction);
        ImGui::ColorEdit4("Color", (float*)&engine.scene.sceneLight.color);
        ImGui::DragFloat4("Debug", (float*)&engine.scene.sceneLight.debugFactors);
        ImGui::Checkbox("Cast Shadows", &engine.scene.sceneLight.castShadows);
        ImGui::End();
        renderInstance();
        /*ImGui::Begin("Node Editor");
        editor.draw();
        ImGui::End();*/
        ImGui::Begin("Asset Browser");
        
        for (auto & pair : engine.scene.geometry) {
        const char * name = pair.first.c_str();
            if (ImGui::Selectable(name)) {  // Make the text clickable
                // Call your function to spawn a new instance here
                selectedInstance = engine.scene.instantiateModel(name, name).entity; 
            }
        }
        ImGui::End();

        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawData, data.commandBuffers[data.currentImgIndex]);
        ECS_UI::currentEntity = selectedInstance;

}
