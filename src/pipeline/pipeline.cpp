#include "pipeline.h"

namespace Rain {
VkResult Pipeline::Init(VkDevice device, const VkExtent2D& extent,
                        VkRenderPass render_pass) {
  shader_ = new Shader;
  VkResult result;
  result = shader_->Init(device, "triangle");
  if (result != VK_SUCCESS) return result;

  const VkShaderStageFlagBits stage_map[Shader::SHADER_STAGE_NUM] = {
      VK_SHADER_STAGE_VERTEX_BIT,
      VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
      VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
      VK_SHADER_STAGE_GEOMETRY_BIT,
      VK_SHADER_STAGE_FRAGMENT_BIT,
      VK_SHADER_STAGE_COMPUTE_BIT,
      VK_SHADER_STAGE_RAYGEN_BIT_NV,
      VK_SHADER_STAGE_ANY_HIT_BIT_NV,
      VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV,
      VK_SHADER_STAGE_MISS_BIT_NV,
      VK_SHADER_STAGE_INTERSECTION_BIT_NV,
      VK_SHADER_STAGE_CALLABLE_BIT_NV,
      VK_SHADER_STAGE_TASK_BIT_NV,
      VK_SHADER_STAGE_MESH_BIT_NV};
  std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos;
  for (size_t i = 0; i < Shader::SHADER_STAGE_NUM; ++i) {
    if (shader_->modules_[i] == VK_NULL_HANDLE) continue;
    VkPipelineShaderStageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    create_info.stage = stage_map[i];
    create_info.module = shader_->modules_[i];
    create_info.pName = "main";
    shader_stage_infos.push_back(create_info);
  }

  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 0;
  vertex_input_info.pVertexBindingDescriptions = nullptr;
  vertex_input_info.vertexAttributeDescriptionCount = 0;
  vertex_input_info.pVertexAttributeDescriptions = nullptr;

  VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
  input_assembly_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly_info.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)extent.width;
  viewport.height = (float)extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = extent;
  VkPipelineViewportStateCreateInfo viewport_info{};
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = &viewport;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer_info{};
  rasterizer_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer_info.depthClampEnable = VK_FALSE;  // TODO: this is a GPU feature
  rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
  rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;  // TODO: wireframe
  rasterizer_info.lineWidth = 1.0f;  // TODO: this is a GPU feature
  rasterizer_info.cullMode =
      VK_CULL_MODE_BACK_BIT;  // TODO: need to change in 3D
  rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer_info.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling_info{};  // Anti-aliasing
  multisampling_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling_info.sampleShadingEnable = VK_FALSE;
  multisampling_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  // TODO: Depth buffer

  VkPipelineColorBlendAttachmentState color_blend_attachment{};
  color_blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable = VK_FALSE;
  VkPipelineColorBlendStateCreateInfo color_blend_info{};
  color_blend_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blend_info.logicOpEnable = VK_FALSE;
  color_blend_info.attachmentCount = 1;
  color_blend_info.pAttachments = &color_blend_attachment;

  VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                     VK_DYNAMIC_STATE_LINE_WIDTH};
  VkPipelineDynamicStateCreateInfo dynamic_state_info{};
  dynamic_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_info.dynamicStateCount = 2;
  dynamic_state_info.pDynamicStates = dynamic_states;

  VkPipelineLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layout_info.setLayoutCount = 0;
  result = vkCreatePipelineLayout(device, &layout_info, nullptr, &layout_);
  if (result != VK_SUCCESS) {
    spdlog::error("pipeline layout creation failed");
    return result;
  }
  spdlog::debug("pipeline layout created");

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = shader_stage_infos.size();
  pipeline_info.pStages = shader_stage_infos.data();
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly_info;
  pipeline_info.pViewportState = &viewport_info;
  pipeline_info.pRasterizationState = &rasterizer_info;
  pipeline_info.pMultisampleState = &multisampling_info;
  pipeline_info.pDepthStencilState = nullptr;
  pipeline_info.pColorBlendState = &color_blend_info;
  pipeline_info.pDynamicState = nullptr;  // TODO
  pipeline_info.layout = layout_;
  pipeline_info.renderPass = render_pass;
  pipeline_info.subpass = 0;

  result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info,
                                     nullptr, &pipeline_);
  if (result != VK_SUCCESS) {
    spdlog::error("pipeline creation failed");
    return result;
  }

  if (shader_) {
    shader_->Destroy(device);
    delete shader_;
  }
  return VK_SUCCESS;
}

void Pipeline::Destroy(VkDevice device) {
  if (layout_ != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(device, layout_, nullptr);
    spdlog::debug("pipeline layout destroyed");
  }
  if (pipeline_ != VK_NULL_HANDLE) {
    vkDestroyPipeline(device, pipeline_, nullptr);
    spdlog::debug("pipeline destroyed");
  }
}
};  // namespace Rain