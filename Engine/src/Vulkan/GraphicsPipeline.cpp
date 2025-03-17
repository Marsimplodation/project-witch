
#include "VkBootstrap.h"
#include "VkRenderer.h"
#include "types/defines.h"
#include <iostream>
#include <fstream>
#include <vulkan/vulkan_core.h>
std::vector<char> VkRenderer::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        printf("failed to open file: %s!", filename.c_str());
        exit(1);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

    file.close();

    return buffer;
}

VkShaderModule VkRenderer::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const u32*>(code.data());

    VkShaderModule shaderModule;
    if (init.disp.createShaderModule(&createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return shaderModule;
}

void VkRenderer::destroyShaderModules(const VkRenderer::ShaderStage* stages) {
    for (int i = 0; i < 2; i++) {
        init.disp.destroyShaderModule(stages[i].module, nullptr);
    }
}

void VkRenderer::createShaderStages(VkRenderer::ShaderStage* stages, const std::string& vert, const std::string& frag) {
    auto vertCode = readFile(vert);
    auto fragCode = readFile(frag);

    stages[0].module = createShaderModule(vertCode);
    stages[1].module = createShaderModule(fragCode);

    if (stages[0].module == VK_NULL_HANDLE || stages[1].module == VK_NULL_HANDLE) {
        std::cerr << "Failed to create shader module\n";
    }

    stages[0].info = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
                       VK_SHADER_STAGE_VERTEX_BIT, stages[0].module, "main", nullptr };
    stages[1].info = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
                       VK_SHADER_STAGE_FRAGMENT_BIT, stages[1].module, "main", nullptr };
}


void VkRenderer::createGraphicsPipelineLayout() {
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(u32) * 1;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = MAX_FRAMES_IN_FLIGHT;
    pipelineLayoutInfo.pSetLayouts = data.descriptorLayouts;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (init.disp.createPipelineLayout(&pipelineLayoutInfo, nullptr, &data.pipelineLayout) != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline layout\n";
        return;
    }
    pipelineLayoutInfo.pSetLayouts = (VkDescriptorSetLayout*)data.descriptorShadowLayouts;
    if (init.disp.createPipelineLayout(&pipelineLayoutInfo, nullptr, &data.shadowPipelineLayout) != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline layout\n";
        return;
    }
}

VkPipeline VkRenderer::createGraphicsPipeline(
    const VkPipelineShaderStageCreateInfo* shaderStages,
    VkPipelineRasterizationStateCreateInfo rasterizer,
    VkRenderPass renderPass, VkPipelineLayout pipelineLayout) {

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = { 0.0f, 0.0f, static_cast<float>(init.swapchain.extent.width),
                            static_cast<float>(init.swapchain.extent.height), 0.0f, 1.0f };

    VkRect2D scissor = { {0, 0}, init.swapchain.extent };

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;


    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPipelineDynamicStateCreateInfo dynamicInfo = {};
    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicInfo.dynamicStateCount = static_cast<u32>(dynamicStates.size());
    dynamicInfo.pDynamicStates = dynamicStates.data();
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicInfo;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.pDepthStencilState = &depthStencil;

    VkPipeline pipeline;
    if (init.disp.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline\n";
        return VK_NULL_HANDLE;
    }

    return pipeline;
}

int VkRenderer::createRenderingPipeline() {
    VkRenderer::ShaderStage graphicsShaders[2];
    VkRenderer::ShaderStage shadowShaders[2];

    createShaderStages(graphicsShaders, data.vertShaderPath, data.fragShaderPath);
    createShaderStages(shadowShaders, data.shadowVertShaderPath, data.shadowFragShaderPath);
    const VkPipelineShaderStageCreateInfo  graphicsShadersInfos[] = {graphicsShaders[0].info, graphicsShaders[1].info};
    const VkPipelineShaderStageCreateInfo  shadowShadersInfos[] = {shadowShaders[0].info, shadowShaders[1].info};

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo shadowRasterizer = rasterizer;
    shadowRasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;

    createGraphicsPipelineLayout();
    data.graphicsPipeline = createGraphicsPipeline(graphicsShadersInfos, rasterizer, data.renderPass, data.pipelineLayout);
    data.offscreenPipeline = createGraphicsPipeline(graphicsShadersInfos, rasterizer, data.offscreenPass,  data.pipelineLayout);
    data.shadowPipeline = createGraphicsPipeline(shadowShadersInfos, shadowRasterizer, data.shadowPass,  data.shadowPipelineLayout);

    destroyShaderModules(graphicsShaders);
    destroyShaderModules(shadowShaders);

    return (data.graphicsPipeline && data.offscreenPipeline && data.shadowPipeline) ? 0 : -1;
}
