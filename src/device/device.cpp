#include "device.h"
#include "spdlog/spdlog.h"

namespace Rain {
VkResult Device::Init(VkPhysicalDevice physical_device,
                      uint32_t queue_family_index,
                      const std::vector<const char*>* layers) {
  VkDeviceQueueCreateInfo queue_create_info{};
  queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_info.queueFamilyIndex = queue_family_index;
  queue_create_info.queueCount = 1;
  float queue_priority = 1.0f;
  queue_create_info.pQueuePriorities = &queue_priority;

  VkPhysicalDeviceFeatures device_features{};

  VkDeviceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.pQueueCreateInfos = &queue_create_info;
  create_info.queueCreateInfoCount = 1;
  create_info.pEnabledFeatures = &device_features;

  create_info.enabledExtensionCount = 0;
  if (layers) {
    create_info.enabledLayerCount = static_cast<uint32_t>(layers->size());
    create_info.ppEnabledLayerNames = layers->data();
  } else
    create_info.enabledLayerCount = 0;
  VkResult result =
      vkCreateDevice(physical_device, &create_info, nullptr, &device_);
  if (result == VK_SUCCESS)
    vkGetDeviceQueue(device_, queue_family_index, 0, &queue_);
  return result;
}

void Device::Destroy() {
  if (device_) {
    spdlog::debug("logical device destroyed");
    vkDestroyDevice(device_, nullptr);
  }
}
};  // namespace Rain