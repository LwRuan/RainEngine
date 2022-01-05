#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "device/device.h"
#include "device/physicaldevice.h"
#include "surface/swapchain.h"
#include "pipeline/pipeline.h"
#include "vkext/debugutils.h"
#include "framebuffer/framebuffer.h"
#include "renderpass/renderpass.h"

namespace Rain {
class Engine {
 public:
  uint32_t width_ = 800;
  uint32_t height_ = 600;

  GLFWwindow* window_ = nullptr;
  VkInstance instance_ = VK_NULL_HANDLE;
  PhysicalDevice* physical_device_ = nullptr;
  Device* device_ = nullptr;
  VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  SwapChain* swap_chain_ = nullptr;
  RenderPass* render_pass_ = nullptr;
  Pipeline* pipeline_ = nullptr;
  std::vector<Framebuffer> framebuffers_;
  VkCommandPool command_pool_ = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> command_buffers_;

  DebugUtilsEXT* debug_utils_ext_ = nullptr;
  bool enable_validation_layers_;
  const std::vector<const char*> validation_layers_ = {
      "VK_LAYER_KHRONOS_validation"};
  const std::vector<const char*> instance_extensions_ = {};

  Engine();
  void Init();
  void DrawFrame();
  void MainLoop();
  void CleanUp();
  bool CheckValidationLayerSupport();
  std::vector<const char*> GetRequiredExtensions();
};
};  // namespace Rain