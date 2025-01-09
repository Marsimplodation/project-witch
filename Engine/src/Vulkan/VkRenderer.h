#ifndef VK_RENDERER_H
#define VK_RENDERER_H

#include "Editor/Editor.h"
#include "types/types.h"
#include <vector>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>


const int MAX_FRAMES_IN_FLIGHT = 1;
const int MAX_INSTANCES = 1000;
const int MAX_OBJECTS = 1000;
struct VkRenderer {
    struct Init {
        GLFWwindow* window;
        vkb::Instance instance;
        vkb::InstanceDispatchTable inst_disp;
        VkSurfaceKHR surface;
        vkb::Device device;
        vkb::PhysicalDevice physicalDevice;
        vkb::DispatchTable disp;
        vkb::Swapchain swapchain;
    }Map;
    struct Texture {
        VkImage image;
        VkImageView view;
        VkDeviceMemory memory;
        VkSampler sampler;
    };

    struct {
        bool recreate = false;
        VkPolygonMode mode = VK_POLYGON_MODE_LINE;
    } renderingMode;

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
        std::vector<Texture> textures;


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
    Init init{};
    RenderData data;

    GLFWwindow* create_window_glfw(const char* window_name = "", bool resize = true);
    void destroy_window_glfw(GLFWwindow* window);
    VkSurfaceKHR create_surface_glfw(VkInstance instance, GLFWwindow* window, VkAllocationCallbacks* allocator = nullptr);
    int device_initialization();
    int create_swapchain();
    int get_queues();
    std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& data);
    int create_graphics_pipeline(VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL);
    int recreate_graphics_pipeline(VkPolygonMode newMode);
    int draw_frame();
    void cleanup();

    ///----- Buffers -----//
    int createGeometryBuffers();
    int createUniformBuffers();
    void updateCameraBuffer();
    void updateModelBuffer(std::vector<glm::mat4> & matrices);
    void destroyBuffers();
    void copyDataToBufferWithStaging(VkBuffer buffer, const void* data, VkDeviceSize bufferSize);
    void copyDataToBuffer(VkDeviceMemory buffer, const void* data, VkDeviceSize bufferSize);
    VkResult createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                  VkBuffer* buffer, VkDeviceMemory* bufferMemory);
    void updateCameraBuffer(Camera & camera);
    u32 findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties);

    //--- Descriptors ----//
    int create_descriptor_pool();
    int create_descriptor_layout(); 
    int update_descriptor_sets();

    //--- Command Buffer ---//
    int record_command_buffer(int i);
    int create_command_pool();
    int create_command_buffers();
    void scene_offscreen_rendering(int i);
    void scene_onscreen_rendering(int i);
    void ui_onscreen_rendering(int i);
    void renderModels(int i);

    //--- Presentation ----//
    int create_off_screen_render_pass();
    void createDepthResources();
    int create_framebuffers();
    int create_sync_objects();
    int recreate_swapchain();
    int createRenderPass();
    //-- Images ---//
    int createImageSampler();
    void createImage(VkImage & image, VkImageView & view, VkDeviceMemory & memory,
                     VkFormat requestedFormat, VkExtent2D extent,
                    VkImageUsageFlags flags, VkImageAspectFlags aspectFlags);
    VkFormat findSupportedFormat(vkb::PhysicalDevice & physicalDevice,
                                 const std::vector<VkFormat>& candidates,
                                 VkImageTiling tiling, VkFormatFeatureFlags features);
    int loadTexture(std::string path);
    void copyDataToImage(VkImage image,const std::vector<glm::vec4>& textureData, VkExtent2D extent, VkDeviceSize dataSize);
    void TransitionImageLayout(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkPipelineStageFlags srcStage,
        VkPipelineStageFlags dstStage);

    };
#endif // !VK_RENDERER_H
//
