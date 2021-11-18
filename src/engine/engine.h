#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "device/physicaldevice.h"
#include "device/device.h"
#include "vkext/debugutils.h"

namespace Rain {
class Engine {
 public:

  uint32_t width_ = 800;
  uint32_t height_ = 600;
  GLFWwindow* window = nullptr;

  VkInstance instance_ = VK_NULL_HANDLE;

  PhysicalDevice* physical_deivce_ = nullptr;
  Device* device_ = nullptr;

  DebugUtilsEXT* debug_utils_ext_ = nullptr;
  bool enable_validation_layers_;
  const std::vector<const char*> validation_layers_ = {
      "VK_LAYER_KHRONOS_validation"};
  const std::vector<const char*> extensions_ = {};

  Engine();
  void Init();
  void MainLoop();
  void CleanUp();
  bool CheckValidationLayerSupport();
  std::vector<const char*> GetRequiredExtensions();
};
};  // namespace Rain