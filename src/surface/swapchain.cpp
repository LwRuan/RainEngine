#include "swapchain.h"

namespace Rain {

VkResult SwapChain::Init(Device* device, PhysicalDevice* physical_device,
                         GLFWwindow* window, VkSurfaceKHR surface) {
  device_ = device;
  VkSurfaceFormatKHR surface_format = physical_device->ChooseSurfaceFormat();
  VkPresentModeKHR present_mode = physical_device->ChoosePresentMode();
  VkExtent2D extent = physical_device->ChooseSwapExtent(window);
  uint32_t min_image_count =
      physical_device->swap_chain_support_details_.capabilities_.minImageCount;
  uint32_t max_image_count =
      physical_device->swap_chain_support_details_.capabilities_.maxImageCount;
  uint32_t image_count = min_image_count + 1;
  if (max_image_count > 0 && image_count > max_image_count) {
    image_count = max_image_count;
  }
  spdlog::debug("swap chain image count: {}", image_count);
  VkSwapchainCreateInfoKHR create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = surface;
  create_info.minImageCount = image_count;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  uint32_t queue_family[2];
  physical_device->GetGraphicsPresentQueueFamily(queue_family[0],
                                                 queue_family[1]);
  if (queue_family[0] != queue_family[1]) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
  }

  create_info.preTransform = physical_device->swap_chain_support_details_
                                 .capabilities_.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;
  create_info.oldSwapchain = VK_NULL_HANDLE;
  VkResult result = vkCreateSwapchainKHR(device->device_, &create_info, nullptr,
                                         &swap_chain_);
  if (result == VK_SUCCESS) {
    vkGetSwapchainImagesKHR(device->device_, swap_chain_, &image_count,
                            nullptr);
    images_.resize(image_count);
    vkGetSwapchainImagesKHR(device->device_, swap_chain_, &image_count,
                            images_.data());
    image_format_ = surface_format.format;
    extent_ = extent;
  }
  return result;
}

void SwapChain::Destroy() {
  if (swap_chain_) {
    vkDestroySwapchainKHR(device_->device_, swap_chain_, nullptr);
    spdlog::debug("swap chain destroyed");
  }
}
};  // namespace Rain