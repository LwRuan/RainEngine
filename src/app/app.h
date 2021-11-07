#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "vkext/debugutils.h"

namespace Rain {
class App {
 public:
  VkInstance instance_ = nullptr;
  DebugUtilsEXT* debug_utils_ext_ = nullptr;
  bool enable_validation_layers_;
  const std::vector<const char*> validation_layers_ = {
      "VK_LAYER_KHRONOS_validation"};
  const std::vector<const char*> extensions_ = {};

  App() {
#ifdef NDEBUG
    enable_validation_layers_ = false;
#else
    enable_validation_layers_ = true;
#endif
  }
  void Init();
  void CleanUp();
  bool CheckValidationLayerSupport();
  std::vector<const char*> GetRequiredExtensions();

};
};  // namespace Rain