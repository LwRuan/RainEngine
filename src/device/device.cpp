#include "device.h"

#include <set>

#include "spdlog/spdlog.h"

namespace Rain {
VkResult Device::Init(VkPhysicalDevice physical_device,
                      uint32_t graphics_queue_family_index,
                      uint32_t present_queue_family_index,
                      const std::vector<const char*>* layers) {
  spdlog::debug("queue family {} picked for graphics", graphics_queue_family_index);
  spdlog::debug("queue family {} picked for present", present_queue_family_index);
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
  }
  return result;
}

void Device::Destroy() {
  if (device_) {
    spdlog::debug("logical device destroyed");
    vkDestroyDevice(device_, nullptr);
  }
}
};  // namespace Rain