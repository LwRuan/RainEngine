#include "descriptor.h"

namespace Rain {
VkResult Descriptors::Init(Device* device, uint32_t n_uniform_vertex,
                           uint32_t n_uniform_fragment, uint32_t n_sampler,
                           uint32_t n_swap_image) {
  // TODO!: sampler not supported
  VkResult result;
  uint32_t n_uniform = n_uniform_vertex + n_uniform_fragment;
  std::vector<VkDescriptorSetLayoutBinding> uniform_bindings;
  uniform_bindings.resize(n_uniform);
  uint32_t idx = 0;
  for (uint32_t i = 0; i < n_uniform_vertex; ++i) {
    uniform_bindings[idx].binding = idx;
    uniform_bindings[idx].descriptorCount = 1;
    uniform_bindings[idx].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_bindings[idx].pImmutableSamplers = nullptr;
    uniform_bindings[idx].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ++idx;
  }
  for (uint32_t i = 0; i < n_uniform_fragment; ++i) {
    uniform_bindings[idx].binding = idx;
    uniform_bindings[idx].descriptorCount = 1;
    uniform_bindings[idx].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;  // image + sampler
    uniform_bindings[idx].pImmutableSamplers = nullptr;
    uniform_bindings[idx].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    ++idx;
  }

  VkDescriptorSetLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = n_uniform;
  layout_info.pBindings = uniform_bindings.data();
  result = vkCreateDescriptorSetLayout(device->device_, &layout_info, nullptr,
                                       &layout_);
  if (result != VK_SUCCESS) {
    spdlog::error("desciptor set layout creation failed");
    return result;
  }

  std::vector<VkDescriptorPoolSize> pool_sizes;
  if (n_uniform > 0) {
    VkDescriptorPoolSize pool_size;
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = n_uniform * n_swap_image;  // TODO:include n_model
    pool_sizes.push_back(pool_size);
  }
  VkDescriptorPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = (uint32_t)pool_sizes.size();
  pool_info.pPoolSizes = pool_sizes.data();
  pool_info.maxSets = n_swap_image;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  result = vkCreateDescriptorPool(device->device_, &pool_info, nullptr, &pool_);
  if(result != VK_SUCCESS) {
    return result;
  }

  std::vector<VkDescriptorSetLayout> layouts(n_swap_image, layout_);
  VkDescriptorSetAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = pool_;
  alloc_info.descriptorSetCount = (uint32_t)n_swap_image; //TODO
  alloc_info.pSetLayouts = layouts.data();
  sets_.resize(n_swap_image);
  if(vkAllocateDescriptorSets(device->device_, &alloc_info, sets_.data()) != VK_SUCCESS) {
    return result;
  }

  return VK_SUCCESS;
}

void Descriptors::Destroy(VkDevice device) {
  if (layout_ != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device, layout_, nullptr);
  }
  if(pool_ != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(device, pool_, nullptr);
  }
}
};  // namespace Rain