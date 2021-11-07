#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "vkext/debugutils.h"

namespace Rain {
class App {
 public:
  uint32_t width_ = 800;
  uint32_t height_ = 600;
  GLFWwindow* window = nullptr;

  VkInstance instance_ = nullptr;
  DebugUtilsEXT* debug_utils_ext_ = nullptr;
  bool enable_validation_layers_;
  const std::vector<const char*> validation_layers_ = {
      "VK_LAYER_KHRONOS_validation"};
  const std::vector<const char*> extensions_ = {};

  App();
  void Init();
  void MainLoop();
  void CleanUp();
  bool CheckValidationLayerSupport();
  std::vector<const char*> GetRequiredExtensions();
};
};  // namespace Rain