#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include <vector>

#include "device/device.h"
#include "device/physicaldevice.h"

namespace Rain {
class SwapChain {
 public:
  Device* device_ = nullptr;
  VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;

  VkFormat image_format_;
  VkExtent2D extent_;
  std::vector<VkImage> images_;
  std::vector<VkImageView> image_views_;

  // VkSemaphore image_available_semaphore_ = VK_NULL_HANDLE;
  // VkSemaphore render_finished_semaphore_ = VK_NULL_HANDLE;
  const int MAX_FRAMES_IN_FLIGHT = 2;
  std::vector<VkSemaphore> image_available_semaphores_;
  std::vector<VkSemaphore> render_finished_semaphores_;
  std::vector<VkFence> in_flight_fences_;
  std::vector<VkFence> images_in_flight_;
  size_t current_frame_ = 0;

  VkResult Init(Device* device, PhysicalDevice* physical_device,
                GLFWwindow* window_, VkSurfaceKHR surface);
  VkResult CreateImageViews();
  uint32_t BeginFrame();
  VkResult EndFrame(VkCommandBuffer* command_buffer, VkQueue graphic_queue,
                    VkQueue present_queue, uint32_t image_index);
  void Destroy();
};
};  // namespace Rain