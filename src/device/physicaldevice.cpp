#include "physicaldevice.h"

#include <algorithm>
#include <string>
#include <vector>

#include "spdlog/spdlog.h"

namespace Rain {
VkBool32 PhysicalDevice::Init(VkInstance instance, VkSurfaceKHR surface) {
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
    QueueFamilyIndices indices = FindQueueFamilies(device, surface, true);
    if (!indices.IsComplete()) device_scores[i] = -1;
  }
  int idx_device = std::distance(
      device_scores.begin(),
      std::max_element(device_scores.begin(), device_scores.end()));
  if (device_scores[idx_device] < 0)  // no queue family supported
    return VK_FALSE;
  device_ = physical_devices[idx_device];
  queue_family_indices_ = FindQueueFamilies(device_, surface, false);
  spdlog::info("GPU{} picked", idx_device);
  return VK_SUCCESS;
}

void PhysicalDevice::GetGraphicsPresentQueueFamily(
    uint32_t& graphics_queue_family, uint32_t& present_queue_family) {
  std::vector<uint32_t> intersect_family{};
  std::set_intersection(queue_family_indices_.graphics_family_.begin(),
                        queue_family_indices_.graphics_family_.end(),
                        queue_family_indices_.present_family_.begin(),
                        queue_family_indices_.present_family_.end(),
                        std::back_inserter(intersect_family));
  if(intersect_family.size()) {
    graphics_queue_family = intersect_family[0];
    present_queue_family = graphics_queue_family;
  }
  else {
    graphics_queue_family = *queue_family_indices_.graphics_family_.begin();
    present_queue_family = *queue_family_indices_.present_family_.begin();
  }
}

PhysicalDevice::QueueFamilyIndices PhysicalDevice::FindQueueFamilies(
    VkPhysicalDevice device, VkSurfaceKHR surface, bool verbose) {
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
      indices.graphics_family_.insert(i);
    }
    if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
      indices.compute_family_.insert(i);
      flags += " compute";
    }
    if (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
      flags += " transfer";
    }
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
    if (present_support) {
      indices.present_family_.insert(i);
      flags += " present";
    }
    if (verbose)
      spdlog::debug(" QueueFamily{} : count {},{}", i, queue_family.queueCount,
                    flags);
  }
  return indices;
}
};  // namespace Rain