#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "buffer/buffer.h"
#include "camera/camera.h"
#include "device/device.h"
#include "mathtype.h"
#include "scene/scene.h"
#include "surface/swapchain.h"
#include "tetmesh.h"

namespace Rain {
struct GlobalUniformData {
  Mat4f proj_view;  // camera
};

struct ModelUniformData {
  Vec4f Ka_d_ = Vec4f(0.2f, 0.2f, 0.2f, 1.0f);   // ambient + non-transparency
  Vec4f Kd_ = Vec4f(0.8f, 0.8f, 0.8f, 0.0f);     // diffuse
  Vec4f Ks_Ns_ = Vec4f(1.0f, 1.0f, 1.0f, 0.0f);  // specular rgb + shininess
  Mat4f model_;
};

class RenderModel {
  // TODO: not optimal, use vma to alloc a big buffer, then divide to models
 public:
  Object* obj_;
  std::vector<Buffer> vertex_buffers_;
  Buffer index_buffer_;
  std::vector<VkBuffer> vertex_vkbuffers_;
  std::vector<VkDeviceSize> vertex_vkbuffer_offsets_;

  VkResult CreateBuffers(Device* device);
  void Destroy(VkDevice device);
  static std::vector<VkVertexInputBindingDescription> GetBindDescription();
  static std::vector<VkVertexInputAttributeDescription>
  GetAttributeDescriptions();
};

class RenderScene {
 public:
  Camera* camera_ = nullptr;
  std::vector<Buffer> global_ubs_;
  std::vector<RenderModel> models_;
  uint32_t n_swap_image_;
  uint32_t n_uniform_buffer_ = 1;  // TODO: now only global
  uint32_t n_uniform_texture_ = 0;

  VkDescriptorSetLayout layout_ = VK_NULL_HANDLE;
  VkDescriptorPool pool_ = VK_NULL_HANDLE;
  std::vector<VkDescriptorSet> sets_;

  VkResult Init(Device* device, SwapChain* swap_chain, Scene* scene);
  VkResult InitUniform(Device* device, SwapChain* swap_chain);
  VkResult InitDescriptor(Device* device);
  void UpdateUniform(VkDevice device, uint32_t image_index);
  void BindAndDraw(VkCommandBuffer command_buffer, VkPipelineLayout layout, uint32_t image_index, uint32_t model_index);
  void DestroyUniform(VkDevice device);
  void Destroy(VkDevice device);
};
};  // namespace Rain