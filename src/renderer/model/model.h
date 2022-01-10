#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "buffer/buffer.h"
#include "device/device.h"
#include "mathtype.h"
#include "tetmesh.h"

namespace Rain {
class Model {
  // TODO: not optimal, use vma to alloc a big buffer, then divide to models
 public:
  uint64_t n_vert_ = 0;
  Vec3f* vertices_ = nullptr;
  Vec3f* colors_ = nullptr;

  std::vector<Buffer> vertex_buffers_;
  std::vector<VkBuffer> vertex_vkbuffers_;
  std::vector<VkDeviceSize> vertex_vkbuffer_offsets_;

  VkResult CreateBuffers(Device* device);
  void Destroy(VkDevice device);
  static std::vector<VkVertexInputBindingDescription> GetBindDescription();
  static std::vector<VkVertexInputAttributeDescription>
  GetAttributeDescriptions();
};
};  // namespace Rain