#ifndef VK_RENDERER_H
#define VK_RENDERER_H

#include "Editor/Editor.h"
#include "types/types.h"
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>

namespace VkRenderer {

const int MAX_FRAMES_IN_FLIGHT = 1;

struct Init {
    GLFWwindow* window;
    vkb::Instance instance;
    vkb::InstanceDispatchTable inst_disp;
    VkSurfaceKHR surface;
    vkb::Device device;
    vkb::PhysicalDevice physicalDevice;
    vkb::DispatchTable disp;
    vkb::Swapchain swapchain;
};

struct RenderData {
    VkQueue graphics_queue;
    VkQueue present_queue;

    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    std::vector<VkFramebuffer> framebuffers;

    std::vector<VkImage> offscreen_images;
    std::vector<VkDeviceMemory> offscreen_image_memory;
    std::vector<VkImageView> offscreen_image_views;
    std::vector<VkFramebuffer> offscreen_framebuffers;


    VkRenderPass render_pass;
    VkRenderPass offscreen_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
    VkPipeline offscreen_pipeline;
    VkSampler imageSampler;
    bool swapchain_out_of_date;
    bool editorMode;
    
    ImguiModule gui;

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkSemaphore> available_semaphores;
    std::vector<VkSemaphore> finished_semaphore;
    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> image_in_flight;

    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
    VkDescriptorSetLayout descriptorLayouts[MAX_FRAMES_IN_FLIGHT];
    VkDescriptorPool descriptorPool;
    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkDeviceMemory indexBufferMemory;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    std::vector<Model> models;
    std::vector<Vertex> * vertices;
    std::vector<u32> * indices;

    std::vector<std::pair<VkBuffer, VkDeviceMemory>> uniformBuffers;



    const char * vertShaderPath;
    const char * fragShaderPath;

    size_t current_frame = 0;
    int current_img_index = 0;
};

GLFWwindow* create_window_glfw(const char* window_name = "", bool resize = true);
void destroy_window_glfw(GLFWwindow* window);
VkSurfaceKHR create_surface_glfw(VkInstance instance, GLFWwindow* window, VkAllocationCallbacks* allocator = nullptr);
int device_initialization(Init& init);
int create_swapchain(Init& init);
int get_queues(Init& init, RenderData& data);
int create_render_pass(Init& init, RenderData& data);
std::vector<char> readFile(const std::string& filename);
VkShaderModule createShaderModule(Init& init, const std::vector<char>& code);
int create_graphics_pipeline(Init& init, RenderData& data);
int create_framebuffers(Init& init, RenderData& data);
int create_sync_objects(Init& init, RenderData& data);
int recreate_swapchain(Init& init, RenderData& data);
int draw_frame(Init& init, RenderData& data);
void cleanup(Init& init, RenderData& data);

///----- Buffers -----//
int createGeometryBuffers(Init& init, RenderData& data);
int createUniformBuffers(Init& init, RenderData& data);
void updateCameraBuffer(Init& init, RenderData& data, Camera & camera);
void updateModelBuffer(Init& init, RenderData& data, Model & model);
void destroyBuffers(Init& init, RenderData& data);
u32 findMemoryType(Init& init, u32 typeFilter, VkMemoryPropertyFlags properties);

//--- Descriptors ----//
int create_descriptor_pool(Init& init, RenderData& data);
int create_descriptor_layout(Init& init, RenderData& data); 
int update_descriptor_sets(Init& init, RenderData& data);

//--- Command Buffer ---//
int record_command_buffer(Init& init, RenderData& data, int i);
int create_command_pool(Init& init, RenderData& data);
int create_command_buffers(Init& init, RenderData& data);

//--- Presentation ----//
int create_off_screen_render_pass(Init& init, RenderData& data);
void createDepthResources(Init& init, RenderData& data);
//-- Images ---//
void createImage(Init& init, RenderData & data,
                 VkImage & image, VkImageView & view, VkDeviceMemory & memory,
                 VkFormat requestedFormat, VkExtent2D extent,
                VkImageUsageFlags flags, VkImageAspectFlags aspectFlags);
VkFormat findSupportedFormat(vkb::PhysicalDevice & physicalDevice,
                             const std::vector<VkFormat>& candidates,
                             VkImageTiling tiling, VkFormatFeatureFlags features);
}
#endif // !VK_RENDERER_H
//
