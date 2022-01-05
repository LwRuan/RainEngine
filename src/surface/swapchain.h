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
  
  VkFormat image_format_;
  VkExtent2D extent_;
  std::vector<VkImage> images_;
  std::vector<VkImageView> image_views_;

  VkResult Init(Device* device, PhysicalDevice* physical_device,
                GLFWwindow* window_, VkSurfaceKHR surface);
  VkResult CreateImageViews();
  void Destroy();
};
};  // namespace Rain