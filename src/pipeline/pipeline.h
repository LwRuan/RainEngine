#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include "shader/shader.h"

namespace Rain {
class Pipeline {
 public:
  Shader* shader_ = nullptr;

  VkResult Init(VkDevice device);
  void Destroy(VkDevice device);
};
};  // namespace Rain