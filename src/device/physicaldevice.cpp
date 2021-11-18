#include "physicaldevice.h"

#include <string>
#include <vector>

#include "spdlog/spdlog.h"

namespace Rain {
VkBool32 PhysicalDevice::Init(VkInstance instance) {
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
  if (device_count == 0) {
    spdlog::error("no avaliable GPU with Vulkan support");
    return VK_FALSE;
  } else
    spdlog::info("{} GPU(s) found", device_count);
  std::vector<VkPhysicalDevice> physical_devices(device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data());
  std::vector<int> device_scores(device_count);
  for (auto i = 0; i < device_count; ++i) {
    const auto& device = physical_devices[i];
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);
    spdlog::info("GPU{}: {}", i, device_properties.deviceName);
    if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      device_scores[i] += 1000;
    }
    QueueFamilyIndices indices = FindQueueFamilies(device, true);
    if (!indices.IsComplete()) device_scores[i] = -1;
  }
  int idx_device = std::distance(
      device_scores.begin(),
      std::max_element(device_scores.begin(), device_scores.end()));
  if (device_scores[idx_device] < 0)  // no queue family supported
    return VK_FALSE;
  device_ = physical_devices[idx_device];
  queue_family_indices_ = FindQueueFamilies(device_, false);
  spdlog::info("GPU{} picked", idx_device);
  return VK_SUCCESS;
}

uint32_t PhysicalDevice::GetRequiredQueueFamily() {
  return queue_family_indices_.graphics_family_.value();
}

PhysicalDevice::QueueFamilyIndices PhysicalDevice::FindQueueFamilies(
    VkPhysicalDevice device, bool verbose) {
  QueueFamilyIndices indices;
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           queue_families.data());
  for (auto i = 0; i < queue_family_count; ++i) {
    const auto& queue_family = queue_families[i];
    std::string flags = "";
    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      flags += " graphics";
      indices.graphics_family_ = i;
    }
    if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
      indices.compute_family_ = i;
      flags += " compute";
    }
    if (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
      flags += " transfer";
    }
    if (verbose)
      spdlog::debug(" QueueFamily{} : count {},{}", i, queue_family.queueCount,
                    flags);
  }
  return indices;
}
};  // namespace Rain