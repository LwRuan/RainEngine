#include "framebuffer.h"

namespace Rain {
VkResult Framebuffer::Init(VkDevice device, const VkExtent2D& extent,
                           VkImageView swap_image_view,
                           VkRenderPass render_pass){
  VkFramebufferCreateInfo framebuffer_info{};
  framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebuffer_info.renderPass = render_pass;
  framebuffer_info.attachmentCount = 1;
  swap_image_view_ = swap_image_view;
  framebuffer_info.pAttachments = &swap_image_view_;
  framebuffer_info.width = extent.width;
  framebuffer_info.height = extent.height;
  framebuffer_info.layers = 1;

  VkResult result = vkCreateFramebuffer(device, &framebuffer_info, nullptr, &framebuffer_);
  if(result != VK_SUCCESS) {
    spdlog::error("framebuffer creation failed");
  }
  return result;
};

void Framebuffer::Destroy(VkDevice device) {
  if(framebuffer_ != VK_NULL_HANDLE) {
    vkDestroyFramebuffer(device, framebuffer_, nullptr);
  }
}
};