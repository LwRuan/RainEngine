#include <vulkan/vulkan.h>
#include <vector>

namespace Rain {
class Device {  // one queue, if want multiple queues, derive this calss
 public:
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue queue_;

  VkResult Init(VkPhysicalDevice physical_device, uint32_t queue_family_index,
                const std::vector<const char*>* layers);
  void Destroy();
};
};  // namespace Rain