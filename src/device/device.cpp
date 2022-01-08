#include "device.h"

#include <set>

#include "spdlog/spdlog.h"

namespace Rain {
VkResult Device::Init(VkPhysicalDevice physical_device,
                      uint32_t graphics_queue_family_index,
                      uint32_t present_queue_family_index,
                      const std::vector<const char*>* layers,
                      const std::vector<const char*>* extensions) {
  spdlog::debug("queue family {} picked for graphics",
                graphics_queue_family_index);
  spdlog::debug("queue family {} picked for present",
                present_queue_family_index);
  std::set<uint32_t> unique_queue_families = {graphics_queue_family_index,
                                              present_queue_family_index};
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  float queue_priority = 1.0f;
  for (uint32_t queue_family : unique_queue_families) {
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = graphics_queue_family_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }
  VkPhysicalDeviceFeatures device_features{};

  VkDeviceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.pQueueCreateInfos = queue_create_infos.data();
  create_info.queueCreateInfoCount =
      static_cast<uint32_t>(queue_create_infos.size());
  create_info.pEnabledFeatures = &device_features;

  if (extensions) {
    create_info.enabledExtensionCount =
        static_cast<uint32_t>(extensions->size());
    create_info.ppEnabledExtensionNames = extensions->data();
  } else
    create_info.enabledExtensionCount = 0;
  if (layers) {
    create_info.enabledLayerCount = static_cast<uint32_t>(layers->size());
    create_info.ppEnabledLayerNames = layers->data();
  } else
    create_info.enabledLayerCount = 0;
  VkResult result =
      vkCreateDevice(physical_device, &create_info, nullptr, &device_);
  if (result == VK_SUCCESS) {
    vkGetDeviceQueue(device_, graphics_queue_family_index, 0, &graphics_queue_);
    vkGetDeviceQueue(device_, present_queue_family_index, 0, &present_queue_);
  } else {
    spdlog::error("logical device creation failed");
    return result;
  }

  {  // create command pool
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = graphics_queue_family_index;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkResult result =
        vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool_);
    if (result != VK_SUCCESS) {
      spdlog::error("command pool creation failed");
      return result;
    } else {
      spdlog::debug("command pool created");
    }
  }
  return VK_SUCCESS;
}

VkResult Device::AllocateCommandBuffers(SwapChain* swap_chain) {
  swap_chain_ = swap_chain;
  command_buffers_.resize(swap_chain_->images_.size());
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool_;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = (uint32_t)command_buffers_.size();
  VkResult result = vkAllocateCommandBuffers(device_, &alloc_info,
                                              command_buffers_.data());
  if (result != VK_SUCCESS) {
    spdlog::error("command buffers allocation failed");
    return result;
  } else {
    spdlog::debug("command buffer allocated");
  }
  return VK_SUCCESS;
}

void Device::Destroy() {
  if (command_pool_ != VK_NULL_HANDLE) {
    vkDestroyCommandPool(device_, command_pool_, nullptr);
    spdlog::debug("command pool destroyed");
  }
  if (device_) {
    vkDestroyDevice(device_, nullptr);
    spdlog::debug("logical device destroyed");
  }
}
};  // namespace Rain