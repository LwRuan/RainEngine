#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cmath>
#include <vector>

#include "camera/camera.h"
#include "descriptor/descriptor.h"
#include "device/device.h"
#include "device/physicaldevice.h"
#include "framebuffer/framebuffer.h"
#include "mathtype.h"
#include "model/model.h"
#include "pipeline/pipeline.h"
#include "renderpass/renderpass.h"
#include "surface/swapchain.h"
#include "vkext/debugutils.h"
#include "steptimer.h"

namespace Rain {
struct GlobalUniformData {
  Mat4f proj_view;  // camera
};
class Engine {
 public:
  GLFWwindow* window_ = nullptr;
  VkInstance instance_ = VK_NULL_HANDLE;
  PhysicalDevice* physical_device_ = nullptr;
  Device* device_ = nullptr;
  VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  SwapChain* swap_chain_ = nullptr;
  RenderPass* render_pass_ = nullptr;
  Descriptors* descriptors_ = nullptr;
  Pipeline* pipeline_ = nullptr;
  std::vector<Framebuffer> framebuffers_;
  Camera* camera_ = nullptr;
  // GlobalUniformData global_data_;
  std::vector<Buffer> global_ubs_;
  StepTimer timer_;

  DebugUtilsEXT* debug_utils_ext_ = nullptr;
  bool enable_validation_layers_;
  const std::vector<const char*> validation_layers_ = {
      "VK_LAYER_KHRONOS_validation"};
  const std::vector<const char*> instance_extensions_ = {};

  uint32_t width_ = 800;
  uint32_t height_ = 600;
  bool window_resized_ = false;

  Model test_triangle_;
  Vec3f vertices_[4] = {{-0.5f, 0.0f, -0.5f},
                        {-0.5f, 0.0f, 0.5f},
                        {0.5f, 0.0f, 0.5f},
                        {0.5f, 0.0f, -0.5f}};
  Vec3f colors_[4] = {{1.0f, 0.0f, 0.0f},
                      {0.0f, 1.0f, 0.0f},
                      {0.0f, 0.0f, 1.0f},
                      {1.0f, 1.0f, 1.0f}};
  uint32_t indices_[6] = {0, 1, 2, 2, 3, 0};

  Engine();
  void Init();
  void DrawFrame();
  void UpdateGlobalUniformBuffer(uint32_t image_index);
  void MainLoop();
  void CleanUp();
  bool CheckValidationLayerSupport();
  std::vector<const char*> GetRequiredExtensions();
  void CleanUpSwapChain();
  void RecreateSwapChain();
  static void WindowResizeCallback(GLFWwindow* window, int width, int height);
};
};  // namespace Rain