#include "engine.h"

#include <cstring>
#include <iostream>

#include "spdlog/spdlog.h"

namespace Rain {
Engine::Engine() {
#ifdef NDEBUG
  enable_validation_layers_ = false;
#else
  enable_validation_layers_ = true;
#endif
}

void Engine::Init() {
  {  // init window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window_ =
        glfwCreateWindow(width_, height_, "Rain Engine", nullptr, nullptr);
  }

  if (enable_validation_layers_) {
    if (!CheckValidationLayerSupport()) {
      spdlog::error("validation layers requested, but not available");
      exit(1);
    }
    debug_utils_ext_ = new DebugUtilsEXT;
  }

  {  // create instance
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Rain Engine";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    if (enable_validation_layers_) {
      create_info.enabledLayerCount =
          static_cast<uint32_t>(validation_layers_.size());
      create_info.ppEnabledLayerNames = validation_layers_.data();
      VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
      DebugUtilsEXT::GetCreateInfo(debug_create_info);
      // create_info.pNext =
      //     (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
    } else {
      create_info.enabledLayerCount = 0;
      create_info.pNext = nullptr;
    }
    auto extensions = GetRequiredExtensions();
    create_info.enabledExtensionCount =
        static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();
    if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
      spdlog::error("instance creation failed");
      CleanUp();
      exit(1);
    } else {
      spdlog::debug("instance created");
    }
  }

  if (enable_validation_layers_) {  // create debug messenger
    if (debug_utils_ext_->Init(instance_) != VK_SUCCESS) {
      spdlog::error("debug messenger creation failed");
      CleanUp();
      exit(1);
    } else
      spdlog::debug("debug messenger created");
  }

  {  // create surface
    VkResult result;
    if ((result = glfwCreateWindowSurface(instance_, window_, nullptr,
                                          &surface_)) != VK_SUCCESS) {
      spdlog::error("{} window surface creation failed", result);
      CleanUp();
      exit(1);
    } else
      spdlog::debug("window surface created");
  }

  {  // create physical device
    physical_device_ = new PhysicalDevice;
    if (physical_device_->Init(instance_, surface_) != VK_SUCCESS) {
      CleanUp();
      exit(1);
    } else
      spdlog::debug("physical device created");
  }

  {  // create logical device
    device_ = new Device;
    VkResult result = VK_SUCCESS;
    uint32_t graphics_queue_family, present_queue_family;
    physical_device_->GetGraphicsPresentQueueFamily(graphics_queue_family,
                                                    present_queue_family);
    if (enable_validation_layers_) {
      result = device_->Init(physical_device_->device_, graphics_queue_family,
                             present_queue_family, &validation_layers_,
                             &physical_device_->device_extensions_);
    } else
      result = device_->Init(physical_device_->device_, graphics_queue_family,
                             present_queue_family, nullptr,
                             &physical_device_->device_extensions_);
    if (result != VK_SUCCESS) {
      spdlog::error("logical device creation failed");
      CleanUp();
      exit(1);
    } else
      spdlog::debug("logical device created");
  }

  {  // create swap chain
    swap_chain_ = new SwapChain;
    if (swap_chain_->Init(device_, physical_device_, window_, surface_) !=
        VK_SUCCESS) {
      spdlog::error("swap chain creation failed");
      CleanUp();
      exit(1);
    } else {
      spdlog::debug("swap chain created");
    }
  }

  {  // create render pass
    render_pass_ = new RenderPass;
    if (render_pass_->Init(device_->device_, swap_chain_->image_format_) !=
        VK_SUCCESS) {
      CleanUp();
      exit(1);
    }
  }

  {  // create pipeline
    pipeline_ = new Pipeline;
    if (pipeline_->Init(device_->device_, swap_chain_->extent_,
                        render_pass_->render_pass_) != VK_SUCCESS) {
      spdlog::error("pipeline creation failed");
      CleanUp();
      exit(1);
    } else {
      spdlog::debug("pipeline created");
    }
  }

  {  // create framebuffers
    framebuffers_.resize(swap_chain_->image_views_.size(), Framebuffer());
    for (size_t i = 0; i < swap_chain_->image_views_.size(); ++i) {
      if (framebuffers_[i].Init(device_->device_, swap_chain_->extent_,
                                swap_chain_->image_views_[i],
                                render_pass_->render_pass_) != VK_SUCCESS) {
        CleanUp();
        exit(1);
      }
    }
    spdlog::debug("framebuffers created");
  }

  {  // create command pool
    uint32_t graphics_queue_family, present_queue_family;
    physical_device_->GetGraphicsPresentQueueFamily(graphics_queue_family,
                                                    present_queue_family);
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = graphics_queue_family;
    VkResult result = vkCreateCommandPool(device_->device_, &pool_info, nullptr,
                                          &command_pool_);
    if (result != VK_SUCCESS) {
      spdlog::error("command pool creation failed");
      CleanUp();
      exit(1);
    } else {
      spdlog::debug("command pool created");
    }
  }

  {  // allocate and record command buffers
    command_buffers_.resize(framebuffers_.size());
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool_;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = (uint32_t)command_buffers_.size();
    VkResult result = vkAllocateCommandBuffers(device_->device_, &alloc_info,
                                               command_buffers_.data());
    if (result != VK_SUCCESS) {
      spdlog::error("command buffers allocation failed");
      CleanUp();
      exit(1);
    } else {
      spdlog::debug("command buffer allocated");
    }
    for (size_t i = 0; i < command_buffers_.size(); ++i) {
      VkCommandBufferBeginInfo begin_info{};
      begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
      ;
      result = vkBeginCommandBuffer(command_buffers_[i], &begin_info);
      if (result != VK_SUCCESS) {
        spdlog::error("command buffer recording start failed");
        CleanUp();
        exit(1);
      }
      VkRenderPassBeginInfo render_pass_info{};
      render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      render_pass_info.renderPass = render_pass_->render_pass_;
      render_pass_info.framebuffer = framebuffers_[i].framebuffer_;
      render_pass_info.renderArea.offset = {0, 0};
      render_pass_info.renderArea.extent = swap_chain_->extent_;
      VkClearValue clear_color = {{{0.2f, 0.2f, 0.2f, 1.0f}}};
      render_pass_info.clearValueCount = 1;
      render_pass_info.pClearValues = &clear_color;
      vkCmdBeginRenderPass(command_buffers_[i], &render_pass_info,
                           VK_SUBPASS_CONTENTS_INLINE);
      vkCmdBindPipeline(command_buffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipeline_->pipeline_);
      vkCmdDraw(command_buffers_[i], 3, 1, 0, 0);
      vkCmdEndRenderPass(command_buffers_[i]);
      if (vkEndCommandBuffer(command_buffers_[i]) != VK_SUCCESS) {
        spdlog::error("command buffer recording failed");
        CleanUp();
        exit(1);
      }
    }
    spdlog::debug("command buffer recorded");
  }
}

void Engine::DrawFrame() {
  uint32_t image_index = swap_chain_->BeginFrame();
  swap_chain_->EndFrame(&command_buffers_[image_index],
                        device_->graphics_queue_, device_->present_queue_,
                        image_index);
}

void Engine::MainLoop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    DrawFrame();
  }
  vkDeviceWaitIdle(device_->device_);
}

void Engine::CleanUp() {
  if (instance_) {
    if (device_) {
      if (command_pool_ != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device_->device_, command_pool_, nullptr);
        spdlog::debug("command pool destroyed");
      }
      if (swap_chain_) {
        swap_chain_->Destroy();
        delete swap_chain_;
      }
      if (pipeline_) {
        pipeline_->Destroy(device_->device_);
        delete pipeline_;
      }
      if (render_pass_) {
        render_pass_->Destroy(device_->device_);
        delete render_pass_;
      }
      for (auto framebuffer : framebuffers_) {
        framebuffer.Destroy(device_->device_);
      }
      spdlog::debug("framebuffers destroyed");
      device_->Destroy();
      delete device_;
    }
    if (debug_utils_ext_) {
      debug_utils_ext_->Destroy(instance_);
      delete debug_utils_ext_;
    }
    if (surface_) {
      spdlog::debug("window surface destroyed");
      vkDestroySurfaceKHR(instance_, surface_, nullptr);
    }
    vkDestroyInstance(instance_, nullptr);
    spdlog::debug("instance destroyed");
  }
  if (physical_device_) delete physical_device_;
  glfwDestroyWindow(window_);
  glfwTerminate();
}

bool Engine::CheckValidationLayerSupport() {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
  for (const char* layer_name : validation_layers_) {
    bool layer_found = false;
    for (const auto& layer_property : available_layers) {
      if (strcmp(layer_name, layer_property.layerName) == 0) {
        layer_found = true;
        break;
      }
    }
    if (!layer_found)
      return false;
    else
      spdlog::debug("{} found", layer_name);
  }
  return true;
}

std::vector<const char*> Engine::GetRequiredExtensions() {
  // functional extensions
  std::vector<const char*> extensions(instance_extensions_);
  // debug extensions
  if (enable_validation_layers_) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  // glfw extensions
  uint32_t glfw_extension_count = 0;
  const char** glfw_extensions =
      glfwGetRequiredInstanceExtensions(&glfw_extension_count);
  extensions.insert(extensions.end(), glfw_extensions,
                    glfw_extensions + glfw_extension_count);
  return extensions;
}

};  // namespace Rain