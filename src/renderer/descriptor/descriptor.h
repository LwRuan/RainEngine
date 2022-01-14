#pragma once

#include <vector>

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include "device/device.h"

namespace Rain {
class Descriptor {
 public:
};

class Descriptors {
  // The better way should be:
  // set 0: global resources
  // set 1: per pass resources
  // set 2: per material resources
  // set 3: per object resources
  // here for simplicity only one set is used
  // includes global and per object resources
  // but in different uniform buffers
 public:
  VkDescriptorSetLayout layout_ = VK_NULL_HANDLE;
  VkDescriptorPool pool_ = VK_NULL_HANDLE;
  std::vector<VkDescriptorSet> sets_;
  
  VkResult Init(Device* device, uint32_t n_uniform_vertex,
                uint32_t n_uniform_fragment, uint32_t n_sampler, uint32_t n_swap_image);
  void Destroy(VkDevice device);
};
};  // namespace Rain