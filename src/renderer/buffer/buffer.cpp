#include "buffer.h"
#include <cstring>

namespace Rain {
VkResult Buffer::Allocate(Device* device, const void* data, uint64_t size,
                          VkBufferUsageFlagBits usage_flags) {
  VkResult result;
  size_ = size;
  VkBufferCreateInfo buffer_info{};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = static_cast<VkDeviceSize>(size);
  buffer_info.usage = usage_flags;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  result = vkCreateBuffer(device->device_, &buffer_info, nullptr, &buffer_);
  if (result != VK_SUCCESS) {
    spdlog::error("buffer allocation failed: size={} code={}", size, result);
    return result;
  }

  VkMemoryRequirements mem_req;
  vkGetBufferMemoryRequirements(device->device_, buffer_, &mem_req);
  properties_ = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VkMemoryAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_req.size;
  alloc_info.memoryTypeIndex =
      device->FindMemoryTypeIndex(mem_req.memoryTypeBits, properties_);
  result = vkAllocateMemory(device->device_, &alloc_info, nullptr, &memory_);
  if (result != VK_SUCCESS) {
    spdlog::error("memory allocation failed");
    return result;
  }

  vkBindBufferMemory(device->device_, buffer_, memory_, 0);

  if(data) {
    void* mapped_ptr;
    vkMapMemory(device->device_, memory_, 0, size_, 0, &mapped_ptr);
    memcpy(mapped_ptr, data, static_cast<uint32_t>(size));
    vkUnmapMemory(device->device_, memory_);
  }

  return VK_SUCCESS;
}

void Buffer::Destroy(VkDevice device) {
  if (buffer_ != VK_NULL_HANDLE) {
    vkDestroyBuffer(device, buffer_, nullptr);
  }
  if (memory_ != VK_NULL_HANDLE) {
    vkFreeMemory(device, memory_, nullptr);
  }
};
};  // namespace Rain