#include "model.h"

namespace Rain {

VkResult Model::CreateBuffers(Device* device) {
  uint64_t size;
  VkResult result;

  vertex_buffers_.resize(2);
  size = (uint64_t)(sizeof(Vec3f)) * n_vert_;
  result = vertex_buffers_[0].Allocate(device, vertices_, size,
                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  if (result != VK_SUCCESS) return result;

  size = (uint64_t)(sizeof(Vec3f)) * n_vert_;
  result = vertex_buffers_[1].Allocate(device, colors_, size,
                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  if (result != VK_SUCCESS) return result;

  vertex_vkbuffers_.clear();
  for (auto buffer : vertex_buffers_) {
    vertex_vkbuffers_.push_back(buffer.buffer_);
  }
  vertex_vkbuffer_offsets_.clear();
  for (int i = 0; i < vertex_buffers_.size(); ++i) {
    vertex_vkbuffer_offsets_.push_back(0);
  }

  return VK_SUCCESS;
}

void Model::Destroy(VkDevice device) {
  for (auto buffer : vertex_buffers_) {
    buffer.Destroy(device);
  }
}

std::vector<VkVertexInputBindingDescription> Model::GetBindDescription() {
  std::vector<VkVertexInputBindingDescription> ret;
  VkVertexInputBindingDescription vertices_desc;
  vertices_desc.binding = 0;
  vertices_desc.stride = sizeof(Vec3f);
  vertices_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  ret.push_back(vertices_desc);

  VkVertexInputBindingDescription colors_desc;
  colors_desc.binding = 1;
  colors_desc.stride = sizeof(Vec3f);
  colors_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  ret.push_back(colors_desc);

  return ret;
}

std::vector<VkVertexInputAttributeDescription>
Model::GetAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> ret;
  VkVertexInputAttributeDescription vertices_desc;
  vertices_desc.binding = 0;
  vertices_desc.location = 0;
  vertices_desc.format = VK_FORMAT_R32G32B32_SFLOAT;
  vertices_desc.offset = 0;
  ret.push_back(vertices_desc);

  VkVertexInputAttributeDescription colors_desc;
  colors_desc.binding = 1;
  colors_desc.location = 1;
  colors_desc.format = VK_FORMAT_R32G32B32_SFLOAT;
  colors_desc.offset = 0;
  ret.push_back(colors_desc);

  return ret;
}
};  // namespace Rain