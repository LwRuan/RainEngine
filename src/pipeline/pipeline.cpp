#include "pipeline.h"

namespace Rain {
VkResult Pipeline::Init(VkDevice device) {
  shader_ = new Shader;
  VkResult result = shader_->Init(device, "triangle");
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
  return VK_SUCCESS;
}

void Pipeline::Destroy(VkDevice device) {
  if (shader_) {
    shader_->Destroy(device);
    delete shader_;
  }
  spdlog::debug("pipeline destroyed");
}
};  // namespace Rain