#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include "device/device.h"
#include "device/physicaldevice.h"

namespace Rain {
class SwapChain {
 public:
  Device* device_ = nullptr;
  VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;
  std::vector<VkImage> images_;
  std::vector<VkImageView> image_views_;
  VkFormat image_format_;
  VkExtent2D extent_;
  VkRenderPass render_pass_;

  VkResult Init(Device* device, PhysicalDevice* physical_device,
                GLFWwindow* window_, VkSurfaceKHR surface);
  VkResult CreateImageViews();
  VkResult CreateRenderPass();
  void Destroy();
};
};  // namespace Rain