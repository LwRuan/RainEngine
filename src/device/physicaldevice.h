#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <set>

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
  QueueFamilyIndices queue_family_indices_;
  VkPhysicalDevice device_ = VK_NULL_HANDLE;

  VkBool32 Init(VkInstance instance, VkSurfaceKHR surface);
  void GetGraphicsPresentQueueFamily(uint32_t& graphics_queue_family, uint32_t& present_queue_family);

  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, bool verbose);
};
};  // namespace Rain