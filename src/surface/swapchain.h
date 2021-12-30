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
  VkFormat image_format_;
  VkExtent2D extent_;

  VkResult Init(Device* device, PhysicalDevice* physical_device,
                GLFWwindow* window_, VkSurfaceKHR surface);
  void Destroy();
};
};  // namespace Rain