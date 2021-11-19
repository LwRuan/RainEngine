#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <set>
#include <vector>

namespace Rain {
class PhysicalDevice {
 public:
  struct QueueFamilyIndices {
    std::set<uint32_t> graphics_family_;
    std::set<uint32_t> compute_family_;
    std::set<uint32_t> present_family_;
    bool IsComplete() {  // the queue family needed
      return (!graphics_family_.empty()) && (!present_family_.empty());
    }
  };

  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities_;
    std::vector<VkSurfaceFormatKHR> formats_;
    std::vector<VkPresentModeKHR> present_modes_;
  };

  QueueFamilyIndices queue_family_indices_;
  SwapChainSupportDetails swap_chain_support_details_;
  VkPhysicalDevice device_ = VK_NULL_HANDLE;
  const std::vector<const char*> device_extensions_ = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkBool32 Init(VkInstance instance, VkSurfaceKHR surface);
  void GetGraphicsPresentQueueFamily(uint32_t& graphics_queue_family,
                                     uint32_t& present_queue_family);

  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device,
                                       VkSurfaceKHR surface, bool verbose);
  bool CheckDeviceExtensionSupport(VkPhysicalDevice device, bool verbose);
  SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device,
                                                VkSurfaceKHR surface,
                                                bool verbose);
};
};  // namespace Rain