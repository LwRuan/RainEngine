#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

namespace Rain {
class Framebuffer {
 public:
  VkImageView swap_image_view_;
  VkFramebuffer framebuffer_ = VK_NULL_HANDLE;

  VkResult Init(VkDevice device, const VkExtent2D& extent,
                VkImageView swap_image_view, VkRenderPass render_pass);
  void Destroy(VkDevice device);
};
};  // namespace Rain