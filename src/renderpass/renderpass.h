#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

namespace Rain {
class RenderPass {
public:
  VkRenderPass render_pass_;

  VkResult Init(VkDevice device, const VkFormat& format);
  void Destroy(VkDevice device);
};
};  // namespace Rain