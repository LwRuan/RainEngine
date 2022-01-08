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
  if (result != VK_SUCCESS) {
    spdlog::error("swap chain creation failed");
    return result;
  }
  vkGetSwapchainImagesKHR(device->device_, swap_chain_, &image_count, nullptr);
  images_.resize(image_count);
  vkGetSwapchainImagesKHR(device->device_, swap_chain_, &image_count,
                          images_.data());
  image_format_ = surface_format.format;
  extent_ = extent;
  result = CreateImageViews();
  if (result != VK_SUCCESS) return result;

  image_available_semaphores_.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
  render_finished_semaphores_.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
  in_flight_fences_.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
  images_in_flight_.resize(images_.size(), VK_NULL_HANDLE);
  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkFenceCreateInfo fence_signaled_info{};
  fence_signaled_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_signaled_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    VkResult result =
        vkCreateSemaphore(device->device_, &semaphore_info, nullptr,
                          &image_available_semaphores_[i]);
    if (result != VK_SUCCESS) {
      spdlog::error("semaphore creation failed");
      return result;
    }
    result = vkCreateSemaphore(device->device_, &semaphore_info, nullptr,
                               &render_finished_semaphores_[i]);
    if (result != VK_SUCCESS) {
      spdlog::error("semaphore creation failed");
      return result;
    }
    result = vkCreateFence(device->device_, &fence_signaled_info, nullptr,
                           &in_flight_fences_[i]);
    if (result != VK_SUCCESS) {
      spdlog::error("fence creation failed");
      return result;
    }
  }

  return VK_SUCCESS;
}

VkResult SwapChain::CreateImageViews() {
  image_views_.resize(images_.size(), VK_NULL_HANDLE);
  for (size_t i = 0; i < images_.size(); ++i) {
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = images_[i];
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = image_format_;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView(device_->device_, &create_info, nullptr,
                                        &image_views_[i]);
    if (result != VK_SUCCESS) {
      spdlog::error("swap chain image view creation failed");
      return result;
    }
  }
  return VK_SUCCESS;
}

uint32_t SwapChain::BeginFrame() {
  vkWaitForFences(device_->device_, 1, &in_flight_fences_[current_frame_],
                  VK_TRUE, UINT64_MAX);
  uint32_t image_index;
  vkAcquireNextImageKHR(device_->device_, swap_chain_, UINT64_MAX,
                        image_available_semaphores_[current_frame_],
                        VK_NULL_HANDLE, &image_index);
  if (images_in_flight_[image_index] != VK_NULL_HANDLE) {
    vkWaitForFences(device_->device_, 1, &images_in_flight_[image_index],
                    VK_TRUE, UINT64_MAX);
  }
  images_in_flight_[image_index] = in_flight_fences_[current_frame_];
  return image_index;
}

VkResult SwapChain::EndFrame(VkCommandBuffer* command_buffer,
                             VkQueue graphic_queue, VkQueue present_queue,
                             uint32_t image_index) {
  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore wait_semaphores[] = {image_available_semaphores_[current_frame_]};
  VkPipelineStageFlags wait_stages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores;
  submit_info.pWaitDstStageMask = wait_stages;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = command_buffer;
  VkSemaphore signal_semaphores[] = {
      render_finished_semaphores_[current_frame_]};
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores;

  vkResetFences(device_->device_, 1, &in_flight_fences_[current_frame_]);
  VkResult result = vkQueueSubmit(graphic_queue, 1, &submit_info,
                                  in_flight_fences_[current_frame_]);
  if (result != VK_SUCCESS) {
    spdlog::error("queue submition failed");
    return result;
  }

  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = signal_semaphores;
  VkSwapchainKHR swap_chains[] = {swap_chain_};
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swap_chains;
  present_info.pImageIndices = &image_index;
  result = vkQueuePresentKHR(present_queue, &present_info);
  if (result != VK_SUCCESS) {
    spdlog::error("queue presentation failed");
    return result;
  }
  current_frame_ = (current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
  return VK_SUCCESS;
}

void SwapChain::Destroy() {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    if (image_available_semaphores_[i] != VK_NULL_HANDLE) {
      vkDestroySemaphore(device_->device_, image_available_semaphores_[i],
                         nullptr);
    }
    if (render_finished_semaphores_[i] != VK_NULL_HANDLE) {
      vkDestroySemaphore(device_->device_, render_finished_semaphores_[i],
                         nullptr);
    }
    if (in_flight_fences_[i] != VK_NULL_HANDLE) {
      vkDestroyFence(device_->device_, in_flight_fences_[i], nullptr);
    }
  }
  if (swap_chain_) {
    for (auto image_view : image_views_) {
      if (image_view != VK_NULL_HANDLE)
        vkDestroyImageView(device_->device_, image_view, nullptr);
    }
    if (swap_chain_ != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(device_->device_, swap_chain_, nullptr);
      spdlog::debug("swap chain destroyed");
    }
  }
}
};  // namespace Rain