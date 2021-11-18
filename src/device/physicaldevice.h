#include <vulkan/vulkan.h>

#include <optional>

namespace Rain {
class PhysicalDevice {
 public:
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family_;
    std::optional<uint32_t> compute_family_;
    bool IsComplete() {  // the queue family needed
      return graphics_family_.has_value();
    }
  };
  QueueFamilyIndices queue_family_indices_;
  VkPhysicalDevice device_ = VK_NULL_HANDLE;

  VkBool32 Init(VkInstance instance);
  uint32_t GetRequiredQueueFamily();

  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, bool verbose);
};
};  // namespace Rain