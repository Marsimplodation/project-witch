#ifndef VK_RENDERER_H
#define VK_RENDERER_H

#include "Editor/Editor.h"
#include "types/defines.h"
#include "types/types.h"
#include <mutex>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>


struct VkRenderer {
    struct Init {
        GLFWwindow* window;
        vkb::Instance instance;
        vkb::InstanceDispatchTable instDisp;
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

    struct RenderData {
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
        std::vector<VkFramebuffer> framebuffers;

        std::vector<VkImage> offscreenImages;
        std::vector<VkDeviceMemory> offscreenImageMemory;
        std::vector<VkImageView> offscreenImageViews;
        std::vector<VkFramebuffer> offscreenFramebuffers;
        std::vector<Texture> textures;
        
        std::vector<std::vector<VkImage>> shadowImages;
        std::vector<std::vector<VkDeviceMemory>> shadowImageMemory;
        std::vector<std::vector<VkImageView>>  shadowImageViews;
        std::vector<std::vector<VkFramebuffer>> shadowFramebuffers;


        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;

        VkRenderPass renderPass;
        VkRenderPass offscreenPass;
        VkRenderPass shadowPass;
        VkPipelineLayout pipelineLayout;
        VkPipelineLayout shadowPipelineLayout;
        VkPipeline graphicsPipeline;
        VkPipeline offscreenPipeline;
        VkPipeline shadowPipeline;
        VkSampler imageSampler;
        bool swapchainOutOfDate;
        bool editorMode;
        
        ImguiModule gui;

        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;

        std::vector<VkSemaphore> availableSemaphores;
        std::vector<VkSemaphore> finishedSemaphore;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imageInFlight;

        VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
        VkDescriptorSet descriptorShadowSets[MAX_FRAMES_IN_FLIGHT];
        VkDescriptorSetLayout descriptorLayouts[MAX_FRAMES_IN_FLIGHT];
        VkDescriptorSetLayout descriptorShadowLayouts[MAX_FRAMES_IN_FLIGHT];
        VkDescriptorPool descriptorPool;
        VkDescriptorPool descriptorShadowPool;
        VkBuffer vertexBuffer;
        VkBuffer indexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkDeviceMemory indexBufferMemory;
        u32 drawCalls;
        u32 instancesRendered;


        std::vector<Vertex> * vertices;
        std::vector<u32> * indices;
        std::vector<Material> materials = std::vector<Material>(MAX_MATERIALS);

        std::vector<std::pair<VkBuffer, VkDeviceMemory>> uniformBuffers;



        const char * vertShaderPath;
        const char * fragShaderPath;
        const char * shadowVertShaderPath;
        const char * shadowFragShaderPath;

        size_t currentFrame = 0;
        int currentImgIndex = 0;
    };
    Init init{};
    RenderData data;

    GLFWwindow* createWindowGlfw(const char* windowName = "", bool resize = true);
    void destroyWindowGlfw(GLFWwindow* window);
    VkSurfaceKHR createSurfaceGlfw(VkInstance instance, GLFWwindow* window, VkAllocationCallbacks* allocator = nullptr);
    int deviceInitialization();
    int createSwapchain();
    int getQueues();
    std::vector<char> readFile(const std::string& filename);

    struct ShaderStage {
        VkShaderModule module;
        VkPipelineShaderStageCreateInfo info;
    };

    VkShaderModule createShaderModule(const std::vector<char>& data);
    void createShaderStages(VkRenderer::ShaderStage * stages, const std::string& vert, const std::string& frag);
    int drawFrame();
    void cleanup();

    ///----- Buffers -----//
    int createGeometryBuffers();
    int createUniformBuffers();
    void updateCameraBuffer();
    void updatematerialBuffer();
    void updateModelBuffer(std::vector<glm::mat4> & matrices);
    void destroyBuffers();
    void copyDataToBufferWithStaging(VkBuffer buffer, const void* data, VkDeviceSize bufferSize);
    void copyDataToBuffer(VkDeviceMemory buffer, const void* data, VkDeviceSize bufferSize);
    VkResult createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                  VkBuffer* buffer, VkDeviceMemory* bufferMemory);
    void updateCameraBuffer(Camera & camera);
    void updateLightBuffer(DirectionLight & light);
    u32 findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties);
    //---Graphics Pipeline--//
    VkPipeline createGraphicsPipeline(
        const VkPipelineShaderStageCreateInfo* shaderStages,
        VkPipelineRasterizationStateCreateInfo rasterizer,
        VkRenderPass renderPass, VkPipelineLayout pipelineLayout
    );
    void createGraphicsPipelineLayout();
    int createRenderingPipeline();
    void destroyShaderModules(const VkRenderer::ShaderStage* stages);


    //--- Descriptors ----//
    int createDescriptorPool();
    int createDescriptorLayout(); 
    int updateDescriptorSets();
    int updateShadowDescriptorSets();

    //--- Command Buffer ---//
    int recordCommandBuffer(int i);
    int createCommandPool();
    int createCommandBuffers();
    void sceneOffscreenRendering(int i);
    void sceneOnscreenRendering(int i);
    void uiOnscreenRendering(int i);
    void sceneShadowRendering(int i);
    void renderModels(int i, int renderingIndex);
    void *enginePtr;

    //--- Presentation ----//
    int createOffScreenRenderPass();
    void createDepthResources();
    int createFramebuffers();
    int createSyncObjects();
    int recreateSwapchain();
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
