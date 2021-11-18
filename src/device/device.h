#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace Rain {
class Device {  // one queue, if want multiple queues, derive this calss
 public:
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue graphics_queue_;
  VkQueue present_queue_;

  VkResult Init(VkPhysicalDevice physical_device,
                uint32_t graphics_queue_family_index,
                uint32_t present_queue_family_index,
                const std::vector<const char*>* layers);
  void Destroy();
};
};  // namespace Rain