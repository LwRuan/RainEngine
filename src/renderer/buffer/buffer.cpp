#include "buffer.h"

#include <cstring>

namespace Rain {
VkResult Buffer::Allocate(Device* device, const void* data, uint64_t size,
                          VkBufferUsageFlagBits usage_flags) {
  size_ = size;
  properties_ = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VkResult result =
      CreateBuffer(device, size, usage_flags, properties_, buffer_, memory_);
  if (result != VK_SUCCESS) return result;

  if (data) {
    void* mapped_ptr;
    vkMapMemory(device->device_, memory_, 0, size_, 0, &mapped_ptr);
    memcpy(mapped_ptr, data, static_cast<uint32_t>(size));
    vkUnmapMemory(device->device_, memory_);
  }

  return VK_SUCCESS;
}

VkResult Buffer::AllocateDeviceLocal(Device* device, const void* data,
                                     uint64_t size,
                                     VkBufferUsageFlagBits usage_flags) {
  VkResult result;
  size_ = size;

  VkBuffer staging_buffer;
  VkDeviceMemory staging_memory;

  result = CreateBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        staging_buffer, staging_memory);
  if (result != VK_SUCCESS) return result;

  if (data) {
    void* mapped_ptr;
    vkMapMemory(device->device_, staging_memory, 0, size, 0, &mapped_ptr);
    memcpy(mapped_ptr, data, static_cast<uint32_t>(size));
    vkUnmapMemory(device->device_, staging_memory);
  }

  VkBufferUsageFlagBits usage_dst = static_cast<VkBufferUsageFlagBits>(
      usage_flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  result = CreateBuffer(device, size, usage_dst,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer_, memory_);
  if (result != VK_SUCCESS) return result;

  CopyBuffer(device, staging_buffer, buffer_, size);
  vkDestroyBuffer(device->device_, staging_buffer, nullptr);
  vkFreeMemory(device->device_, staging_memory, nullptr);
  return VK_SUCCESS;
}

VkResult Buffer::CreateBuffer(Device* device, uint64_t size,
                              VkBufferUsageFlagBits usage_flags,
                              VkMemoryPropertyFlags properties,
                              VkBuffer& buffer, VkDeviceMemory& memory) {
  VkResult result;
  VkBufferCreateInfo buffer_info{};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = static_cast<VkDeviceSize>(size);
  buffer_info.usage = usage_flags;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  result = vkCreateBuffer(device->device_, &buffer_info, nullptr, &buffer);
  if (result != VK_SUCCESS) {
    spdlog::error("buffer allocation failed: size={} code={}", size, result);
    return result;
  }

  VkMemoryRequirements mem_req;
  vkGetBufferMemoryRequirements(device->device_, buffer, &mem_req);

  VkMemoryAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_req.size;
  alloc_info.memoryTypeIndex =
      device->FindMemoryTypeIndex(mem_req.memoryTypeBits, properties);
  result = vkAllocateMemory(device->device_, &alloc_info, nullptr, &memory);
  if (result != VK_SUCCESS) {
    spdlog::error("memory allocation failed");
    return result;
  }

  vkBindBufferMemory(device->device_, buffer, memory, 0);

  return VK_SUCCESS;
}

VkResult Buffer::CopyBuffer(Device* device, VkBuffer src, VkBuffer dst,
                            VkDeviceSize size) {
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandPool = device->command_pool_;
  alloc_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  vkAllocateCommandBuffers(device->device_, &alloc_info, &command_buffer);

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(command_buffer, &begin_info);

  VkBufferCopy copy_region{};
  copy_region.srcOffset = 0;
  copy_region.dstOffset = 0;
  copy_region.size = size;
  vkCmdCopyBuffer(command_buffer, src, dst, 1, &copy_region);
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;
  vkQueueSubmit(device->graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(device->graphics_queue_);
  vkFreeCommandBuffers(device->device_, device->command_pool_, 1,
                       &command_buffer);

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