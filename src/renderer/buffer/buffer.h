#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include "device/device.h"

namespace Rain {
class Buffer {
 public:
  VkBuffer buffer_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  VkDeviceSize	size_;
  VkMemoryPropertyFlags properties_;
  VkResult Allocate(Device* device, const void* data, uint64_t size,
                    VkBufferUsageFlagBits usage_flags);
  void Destroy(VkDevice device);
};
};  // namespace Rain