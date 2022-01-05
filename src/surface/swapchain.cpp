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
  result = CreateRenderPass();
  if (result != VK_SUCCESS) return result;
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

VkResult SwapChain::CreateRenderPass() {
  VkAttachmentDescription color_attachment{};
  color_attachment.format = image_format_;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref{};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;

  VkResult result = vkCreateRenderPass(device_->device_, &render_pass_info,
                                       nullptr, &render_pass_);
  if (result != VK_SUCCESS) {
    spdlog::error("render pass creation failed");
    return result;
  }
  spdlog::debug("render pass created");
  return VK_SUCCESS;
}

void SwapChain::Destroy() {
  if (swap_chain_) {
    for (auto image_view : image_views_) {
      if (image_view != VK_NULL_HANDLE)
        vkDestroyImageView(device_->device_, image_view, nullptr);
    }
    if (render_pass_ != VK_NULL_HANDLE) {
      vkDestroyRenderPass(device_->device_, render_pass_, nullptr);
      spdlog::debug("render pass destroyed");
    }
    if (swap_chain_ != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(device_->device_, swap_chain_, nullptr);
      spdlog::debug("swap chain destroyed");
    }
  }
}
};  // namespace Rain