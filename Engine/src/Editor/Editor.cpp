#include "Editor.h"
#include "Editor/NodeEditor.h"
#include "GLFW/glfw3.h"
#include <cmath>
#include <iterator>
#include "../Vulkan/VkRenderer.h"
#include "ImGuizmo.h"
#include "InputHandling/KeyDefines.h"
#include "Physics/JoltImpl.h"
#include "SoulShard.h"
#include "glm/detail/qualifier.hpp"
#include "glm/fwd.hpp"
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
    pool_info.maxSets = 1000;
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
    init_info.Queue = data.graphics_queue;
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = init.swapchain.image_count;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.RenderPass = data.render_pass;
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

void editorControls(SoulShard & engine) {
    if(engine.inputHandler.isKeyPressedOnce(KEY_F1)) {
        engine.renderer.renderingMode = {.recreate = true, .mode = VK_POLYGON_MODE_LINE};
    }
    if(engine.inputHandler.isKeyPressedOnce(KEY_F2)) {
        engine.renderer.renderingMode = {.recreate = true, .mode = VK_POLYGON_MODE_FILL};
    }
}

Instance * selectedInstance = 0x0;
std::vector<ImTextureID> textures;
void ImguiModule::update(void * initPtr, void * dataPtr) {
   	VkRenderer::Init & init = * (VkRenderer::Init*)initPtr;
    	VkRenderer::RenderData & data = *(VkRenderer::RenderData*) dataPtr;
        SoulShard & engine = *((SoulShard*)enginePtr);
        editorControls(engine);

        if(!active) return;
        if(textureCreated) {
            ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(textureID));
            for(auto & texture : textures)
            ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(texture));
        }
        textures.resize(data.textures.size() + 1);
        for(int i = 0; i < data.textures.size(); ++i) {
            textures[i] = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(
            data.textures[i].sampler,
            data.textures[i].view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL // Image layout for sampling
        ));
        }
        textures[textures.size() -1] = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(
        data.imageSampler,
        data.shadow_image_view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL // Image layout for sampling
        ));
        textureID = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(
            data.imageSampler,
            data.offscreen_image_views[data.current_frame],                       // VkSampler
            VK_IMAGE_LAYOUT_GENERAL // Image layout for sampling
        ));
        textureCreated = true;

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame(); // If using GLFW
        ImGui::NewFrame();// Set up docking
        ImGuizmo::BeginFrame();

        ImGui::DockSpaceOverViewport(0);

        // Build your GUI
        renderViewport(initPtr, dataPtr);        
        ImGui::Begin("Debug");
        ImGui::Text("FPS %f", 1.0f/((SoulShard*)enginePtr)->deltaTime);
        ImGui::Text("Vertices: %s", formatNumberWithDots(data.vertices->size()).c_str());
        ImGui::Text("Triangles: %s", formatNumberWithDots(data.indices->size() / 3).c_str());
        ImGui::End();
        ImGui::Begin("Systems");
        for(auto & system : engine.systems) {
            const char * name = system.name.c_str();
            ImGui::Text("%s", name);
            ImGui::Checkbox((std::string("Active##") + name).c_str(), &system.active);
        }
        ImGui::End();
        ImGui::Begin("Textures");
        ImGui::Image(textures[textures.size() -1], ImVec2(512, 512));
        for(int i = 0; i < data.textures.size(); ++i) {
            ImGui::Image(textures[i], ImVec2(512, 512));
        }
        ImGui::End();



        ImGui::Begin("Scene");
        int instanceIndex = 0;
        for (auto & instance : engine.scene.instances) {
            auto name = instance.name + std::string("##") + std::to_string(instanceIndex++);
            if (ImGui::Selectable(name.c_str())) {  // Make the text clickable
                selectedInstance = &instance;
            }
        }
        ImGui::End();
        ImGui::Begin("Selected");
        if(selectedInstance) {

            auto & instance = *selectedInstance;
            const char * name = instance.name.c_str();
            ImGui::Text("Name: %s", name);
            ImGui::BeginChild("Transform Component");
            glm::mat4 & transform = engine.scene.registry.get<TransformComponent>(instance.entity).mat;
            engine.editorCamera.projection[1][1] *= -1;
            ImGuizmo::Manipulate(glm::value_ptr(engine.editorCamera.view),
                             glm::value_ptr(engine.editorCamera.projection),
                             ImGuizmo::OPERATION::UNIVERSAL,
                             ImGuizmo::WORLD,
                             glm::value_ptr(transform));
            engine.editorCamera.projection[1][1] *= -1;
            glm::vec3 newPosition, newRotation, newScale;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform),
                                                  glm::value_ptr(newPosition),
                                                  glm::value_ptr(newRotation),
                                                  glm::value_ptr(newScale));
            ImGui::DragFloat3("position", (float*)&newPosition, 0.1f);
            ImGui::DragFloat3("rotation", (float*)&newRotation, 0.1f);
            ImGui::DragFloat3("scale", (float*)&newScale, 0.1f);
            ImGui::EndChild();
        }
        ImGui::End();
        
        /*ImGui::Begin("Node Editor");
        editor.draw();
        ImGui::End();*/
        ImGui::Begin("Asset Browser");
        
        for (auto & pair : engine.scene.geometry) {
        const char * name = pair.first.c_str();
            if (ImGui::Selectable(name)) {  // Make the text clickable
                // Call your function to spawn a new instance here
                selectedInstance = &engine.scene.instantiateModel(name, name); 
                createRigidBody(*selectedInstance, engine.scene);
            }
        }
        ImGui::End();

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, data.command_buffers[data.current_img_index]);
}
