#include "Editor.h"
#include "GLFW/glfw3.h"
#include <cmath>
#include <iterator>
#include "../Vulkan/VkRenderer.h"
#include "glm/detail/qualifier.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

void customTheme() {
        auto &colors = ImGui::GetStyle().Colors;

    // General Background
    colors[ImGuiCol_WindowBg] = ImVec4{0.12f, 0.12f, 0.15f, 1.0f};
    colors[ImGuiCol_MenuBarBg] = ImVec4{0.14f, 0.14f, 0.18f, 1.0f};

    // Border
    colors[ImGuiCol_Border] = ImVec4{0.3f, 0.3f, 0.3f, 0.8f};
    colors[ImGuiCol_BorderShadow] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};

    // Text
    colors[ImGuiCol_Text] = ImVec4{0.9f, 0.9f, 0.9f, 1.0f};
    colors[ImGuiCol_TextDisabled] = ImVec4{0.5f, 0.5f, 0.5f, 1.0f};

    // Headers
    colors[ImGuiCol_Header] = ImVec4{0.2f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_HeaderHovered] = ImVec4{0.25f, 0.25f, 0.3f, 1.0f};
    colors[ImGuiCol_HeaderActive] = ImVec4{0.3f, 0.3f, 0.35f, 1.0f};

    // Buttons
    colors[ImGuiCol_Button] = ImVec4{0.2f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.3f, 0.4f, 1.0f};
    colors[ImGuiCol_ButtonActive] = ImVec4{0.35f, 0.35f, 0.45f, 1.0f};
    colors[ImGuiCol_CheckMark] = ImVec4{0.45f, 0.7f, 1.0f, 1.0f}; // Vibrant accent color

    // Popups
    colors[ImGuiCol_PopupBg] = ImVec4{0.1f, 0.1f, 0.14f, 0.95f};

    // Slider
    colors[ImGuiCol_SliderGrab] = ImVec4{0.3f, 0.5f, 1.0f, 0.7f}; // Smooth blue accent
    colors[ImGuiCol_SliderGrabActive] = ImVec4{0.4f, 0.6f, 1.0f, 0.8f};

    // Frame BG
    colors[ImGuiCol_FrameBg] = ImVec4{0.17f, 0.17f, 0.22f, 1.0f};
    colors[ImGuiCol_FrameBgHovered] = ImVec4{0.2f, 0.2f, 0.27f, 1.0f};
    colors[ImGuiCol_FrameBgActive] = ImVec4{0.25f, 0.25f, 0.32f, 1.0f};

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4{0.14f, 0.14f, 0.18f, 1.0f};
    colors[ImGuiCol_TabHovered] = ImVec4{0.3f, 0.3f, 0.4f, 1.0f};
    colors[ImGuiCol_TabActive] = ImVec4{0.25f, 0.25f, 0.35f, 1.0f};
    colors[ImGuiCol_TabUnfocused] = ImVec4{0.14f, 0.14f, 0.18f, 1.0f};
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.2f, 0.27f, 1.0f};

    // Title
    colors[ImGuiCol_TitleBg] = ImVec4{0.14f, 0.14f, 0.18f, 1.0f};
    colors[ImGuiCol_TitleBgActive] = ImVec4{0.2f, 0.2f, 0.27f, 1.0f};
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4{0.12f, 0.12f, 0.15f, 1.0f};
    colors[ImGuiCol_ScrollbarGrab] = ImVec4{0.3f, 0.3f, 0.35f, 1.0f};
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{0.35f, 0.35f, 0.4f, 1.0f};
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{0.4f, 0.4f, 0.45f, 1.0f};

    // Separator
    colors[ImGuiCol_Separator] = ImVec4{0.3f, 0.3f, 0.4f, 1.0f};
    colors[ImGuiCol_SeparatorHovered] = ImVec4{0.4f, 0.4f, 0.5f, 1.0f};
    colors[ImGuiCol_SeparatorActive] = ImVec4{0.5f, 0.5f, 0.6f, 1.0f};

    // Resize Grip
    colors[ImGuiCol_ResizeGrip] = ImVec4{0.3f, 0.3f, 0.4f, 0.6f};
    colors[ImGuiCol_ResizeGripHovered] = ImVec4{0.35f, 0.35f, 0.45f, 0.7f};
    colors[ImGuiCol_ResizeGripActive] = ImVec4{0.4f, 0.4f, 0.5f, 0.8f};

    // Docking
    colors[ImGuiCol_DockingPreview] = ImVec4{0.45f, 0.7f, 1.0f, 0.8f}; // Blue glow effect

    // Style Adjustments for Modern Feel
    auto &style = ImGui::GetStyle();
    style.TabRounding = 6;
    style.ScrollbarRounding = 10;
    style.WindowRounding = 8;
    style.GrabRounding = 4;
    style.FrameRounding = 5;
    style.PopupRounding = 6;
    style.ChildRounding = 6;

    // General padding and spacing adjustments
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);

}

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

}

void ImguiModule::destroy(VkDevice device) {
    // Cleanup (happens once)
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(device, imguiPool, nullptr);
}

bool textureCreated = false;

void ImguiModule::update(void * initPtr, void * dataPtr) {
   	VkRenderer::Init & init = * (VkRenderer::Init*)initPtr;
    	VkRenderer::RenderData & data = *(VkRenderer::RenderData*) dataPtr;

        if(!active) return;
        if(textureCreated) {
            ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(textureID));
        }
        textureID = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(
            data.imageSampler,
            data.offscreen_image_views[data.current_frame],                       // VkSampler
            VK_IMAGE_LAYOUT_GENERAL // Image layout for sampling
        ));
        textureCreated = true;

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame(); // If using GLFW
        ImGui::NewFrame();// Set up docking

        ImGui::DockSpaceOverViewport(0);

        // Build your GUI
        ImGui::Begin("Viewport");
        
        if(ImGui::GetWindowSize().x != previewSize.x || ImGui::GetWindowSize().y != previewSize.y) {
            data.swapchain_out_of_date = true;
        }
        ImGui::Image(textureID,previewSize);
        previewSize = ImGui::GetWindowSize(); 

        ImGui::End();
        
        ImGui::Begin("Systems");
        ImGui::End();

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, data.command_buffers[data.current_img_index]);
}
