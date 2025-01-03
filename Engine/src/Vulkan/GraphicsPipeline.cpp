
#include "VkBootstrap.h"
#include "VkRenderer.h"
#include <iostream>
#include <fstream>
#include <vulkan/vulkan_core.h>
namespace VkRenderer {
    std::vector<char> readFile(const std::string& filename) {
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

    VkShaderModule createShaderModule(Init& init, const std::vector<char>& code) {
        VkShaderModuleCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (init.disp.createShaderModule(&create_info, nullptr, &shaderModule) != VK_SUCCESS) {
            return VK_NULL_HANDLE; // failed to create shader module
        }

        return shaderModule;
    }
    int create_graphics_pipeline(Init& init, RenderData& data) {
        auto vert_code = readFile(data.vertShaderPath);
        auto frag_code = readFile(data.fragShaderPath);

        VkShaderModule vert_module = createShaderModule(init, vert_code);
        VkShaderModule frag_module = createShaderModule(init, frag_code);
        if (vert_module == VK_NULL_HANDLE || frag_module == VK_NULL_HANDLE) {
            std::cout << "failed to create shader module\n";
            return -1; // failed to create shader modules
        }

        VkPipelineShaderStageCreateInfo vert_stage_info = {};
        vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vert_stage_info.module = vert_module;
        vert_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo frag_stage_info = {};
        frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_stage_info.module = frag_module;
        frag_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo shader_stages[] = { vert_stage_info, frag_stage_info };

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

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)init.swapchain.extent.width;
        viewport.height = (float)init.swapchain.extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = init.swapchain.extent;

        VkPipelineViewportStateCreateInfo viewport_state = {};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

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
        color_blending.blendConstants[0] = 0.0f;
        color_blending.blendConstants[1] = 0.0f;
        color_blending.blendConstants[2] = 0.0f;
        color_blending.blendConstants[3] = 0.0f;
        
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional


        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = MAX_FRAMES_IN_FLIGHT;
        pipeline_layout_info.pSetLayouts = data.descriptorLayouts;
        pipeline_layout_info.pushConstantRangeCount = 0;

        if (init.disp.createPipelineLayout(&pipeline_layout_info, nullptr, &data.pipeline_layout) != VK_SUCCESS) {
            std::cout << "failed to create pipeline layout\n";
            return -1; // failed to create pipeline layout
        }

        std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

        VkPipelineDynamicStateCreateInfo dynamic_info = {};
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
        pipeline_info.layout = data.pipeline_layout;
        pipeline_info.renderPass = data.render_pass;
        pipeline_info.subpass = 0;
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_info.pDepthStencilState = &depthStencil;

        if (init.disp.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &data.graphics_pipeline) != VK_SUCCESS) {
            std::cout << "failed to create pipline\n";
            return -1; // failed to create graphics pipeline
        }
        
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
        pipeline_info.layout = data.pipeline_layout;
        pipeline_info.renderPass = data.offscreen_pass;
        pipeline_info.subpass = 0;
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_info.pDepthStencilState = &depthStencil;

        if (init.disp.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &data.offscreen_pipeline) != VK_SUCCESS) {
            std::cout << "failed to create pipline\n";
            return -1; // failed to create graphics pipeline
        }

        init.disp.destroyShaderModule(frag_module, nullptr);
        init.disp.destroyShaderModule(vert_module, nullptr);
        return 0;
    }
}
