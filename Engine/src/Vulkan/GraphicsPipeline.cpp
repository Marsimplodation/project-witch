
#include "VkBootstrap.h"
#include "VkRenderer.h"
#include <iostream>
#include <fstream>
#include <vulkan/vulkan_core.h>
std::vector<char> VkRenderer::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        printf("failed to open file: %s!", filename.c_str());
        exit(1);
    }

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(file_size));

    file.close();

    return buffer;
}

VkShaderModule VkRenderer::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (init.disp.createShaderModule(&create_info, nullptr, &shaderModule) != VK_SUCCESS) {
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
    auto vert_code = readFile(vert);
    auto frag_code = readFile(frag);

    stages[0].module = createShaderModule(vert_code);
    stages[1].module = createShaderModule(frag_code);

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
    pushConstantRange.size = sizeof(u32) * 2;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = MAX_FRAMES_IN_FLIGHT;
    pipeline_layout_info.pSetLayouts = data.descriptorLayouts;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &pushConstantRange;

    if (init.disp.createPipelineLayout(&pipeline_layout_info, nullptr, &data.pipeline_layout) != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline layout\n";
        return;
    }
    pipeline_layout_info.pSetLayouts = data.descriptorShadowLayouts;
    if (init.disp.createPipelineLayout(&pipeline_layout_info, nullptr, &data.shadow_pipeline_layout) != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline layout\n";
        return;
    }
}

VkPipeline VkRenderer::createGraphicsPipeline(
    const VkPipelineShaderStageCreateInfo* shader_stages,
    VkPipelineRasterizationStateCreateInfo rasterizer,
    VkRenderPass renderPass, VkPipelineLayout pipelineLayout) {

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertex_input_info.pVertexBindingDescriptions = &bindingDescription;
    vertex_input_info.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = { 0.0f, 0.0f, static_cast<float>(init.swapchain.extent.width),
                            static_cast<float>(init.swapchain.extent.height), 0.0f, 1.0f };

    VkRect2D scissor = { {0, 0}, init.swapchain.extent };

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &colorBlendAttachment;


    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPipelineDynamicStateCreateInfo dynamic_info = {};
    std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_info.pDynamicStates = dynamic_states.data();
    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_info;
    pipeline_info.layout = pipelineLayout;
    pipeline_info.renderPass = renderPass;
    pipeline_info.subpass = 0;
    pipeline_info.pDepthStencilState = &depthStencil;

    VkPipeline pipeline;
    if (init.disp.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS) {
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
    data.graphics_pipeline = createGraphicsPipeline(graphicsShadersInfos, rasterizer, data.render_pass, data.pipeline_layout);
    data.offscreen_pipeline = createGraphicsPipeline(graphicsShadersInfos, rasterizer, data.offscreen_pass,  data.pipeline_layout);
    data.shadow_pipeline = createGraphicsPipeline(shadowShadersInfos, shadowRasterizer, data.shadow_pass,  data.shadow_pipeline_layout);

    destroyShaderModules(graphicsShaders);
    destroyShaderModules(shadowShaders);

    return (data.graphics_pipeline && data.offscreen_pipeline && data.shadow_pipeline) ? 0 : -1;
}
