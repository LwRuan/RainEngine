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
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, Engine::WindowResizeCallback);
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
    } else {
      spdlog::debug("render pass created");
    }
  }

  {  // global ubo
    VkDeviceSize size = sizeof(GlobalUniformData);
    global_ubs_.resize(swap_chain_->images_.size());
    for (size_t i = 0; i < global_ubs_.size(); ++i) {
      if (global_ubs_[i].Allocate(device_, nullptr, size,
                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) !=
          VK_SUCCESS) {
        CleanUp();
        exit(1);
      };
    }
  }

  {  // create descriptors
    descriptors_ = new Descriptors;
    // 1 for camera data
    if (descriptors_->Init(device_, 1, 0, 0, swap_chain_->images_.size()) !=
        VK_SUCCESS) {
      CleanUp();
      exit(1);
    } else {
      spdlog::debug("descriptors created");
    }
    for(size_t i=0;i<swap_chain_->images_.size();++i) {
      VkDescriptorBufferInfo buffer_info{};
      buffer_info.buffer = global_ubs_[i].buffer_;
      buffer_info.offset = 0;
      buffer_info.range = sizeof(GlobalUniformData);
      VkWriteDescriptorSet descriptor_write{};
      descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptor_write.dstSet = descriptors_->sets_[i];
      descriptor_write.dstBinding = 0;
      descriptor_write.dstArrayElement = 0;
      descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptor_write.descriptorCount = 1;
      descriptor_write.pBufferInfo = &buffer_info;
      vkUpdateDescriptorSets(device_->device_, 1, &descriptor_write, 0, nullptr);
    }
  }

  {  // create pipeline
    pipeline_ = new Pipeline;
    if (pipeline_->Init(device_->device_, swap_chain_->extent_,
                        render_pass_->render_pass_,
                        descriptors_) != VK_SUCCESS) {
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

  // allocate and record command buffers
  device_->AllocateCommandBuffers(swap_chain_);

  {  // model
    test_triangle_.n_vert_ = 4;
    test_triangle_.vertices_ = vertices_;
    test_triangle_.colors_ = colors_;
    test_triangle_.n_ele_ = 2;
    test_triangle_.indices_ = indices_;
    if (test_triangle_.CreateBuffers(device_) != VK_SUCCESS) {
      CleanUp();
      exit(1);
    };
  }

  {  // Camera
    camera_ = new Camera;
    float aspect = 1.0;
    if (swap_chain_->extent_.height)
      aspect = float(swap_chain_->extent_.width) / swap_chain_->extent_.height;
    camera_->InitData(aspect, 0.25f * PI_, 1.0f, 1000.0f, 3.0f, 0.0f,
                      0.3f * PI_, Vec3::Zero());
  }

  timer_.Reset();
}

void Engine::DrawFrame() {
  uint32_t image_index = swap_chain_->BeginFrame(window_resized_);
  while (window_resized_) {
    window_resized_ = false;
    RecreateSwapChain();
    image_index = swap_chain_->BeginFrame(window_resized_);
  }

  UpdateGlobalUniformBuffer(image_index);

  VkCommandBuffer command_buffer = device_->command_buffers_[image_index];
  vkResetCommandBuffer(command_buffer,
                       VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  VkResult result = vkBeginCommandBuffer(command_buffer, &begin_info);
  if (result != VK_SUCCESS) {
    spdlog::error("command buffer recording start failed");
    CleanUp();
    exit(1);
  }
  VkRenderPassBeginInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = render_pass_->render_pass_;
  render_pass_info.framebuffer = framebuffers_[image_index].framebuffer_;
  render_pass_info.renderArea.offset = {0, 0};
  render_pass_info.renderArea.extent = swap_chain_->extent_;
  VkClearValue clear_color = {{{0.6f, 0.6f, 0.6f, 1.0f}}};
  render_pass_info.clearValueCount = 1;
  render_pass_info.pClearValues = &clear_color;
  vkCmdBeginRenderPass(command_buffer, &render_pass_info,
                       VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline_->pipeline_);
  vkCmdBindVertexBuffers(command_buffer, 0,
                         test_triangle_.vertex_vkbuffers_.size(),
                         test_triangle_.vertex_vkbuffers_.data(),
                         test_triangle_.vertex_vkbuffer_offsets_.data());
  vkCmdBindIndexBuffer(command_buffer, test_triangle_.index_buffer_.buffer_, 0,
                       VK_INDEX_TYPE_UINT32);
  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_->layout_, 0, 1, &descriptors_->sets_[image_index], 0, nullptr);
  vkCmdDrawIndexed(command_buffer,
                   static_cast<uint32_t>(test_triangle_.n_ele_ * 3), 1, 0, 0,
                   0);
  vkCmdEndRenderPass(command_buffer);
  if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
    spdlog::error("command buffer recording failed");
    CleanUp();
    exit(1);
  }
  swap_chain_->EndFrame(image_index);
}

void Engine::MainLoop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    DrawFrame();
  }
  vkDeviceWaitIdle(device_->device_);
}

void Engine::UpdateGlobalUniformBuffer(uint32_t image_index) {
  void* data;
  vkMapMemory(device_->device_, global_ubs_[image_index].memory_, 0,
              sizeof(GlobalUniformData), 0, &data);
  memcpy(data, &camera_->proj_view_, sizeof(Mat4f));
  vkUnmapMemory(device_->device_, global_ubs_[image_index].memory_);
}

void Engine::CleanUp() {
  for (auto buffer : global_ubs_) {
    buffer.Destroy(device_->device_);
  }
  test_triangle_.Destroy(device_->device_);
  if (camera_) {
    delete camera_;
  }
  if (descriptors_) {
    descriptors_->Destroy(device_->device_);
    spdlog::debug("descriptors destroyed");
    delete descriptors_;
  }
  if (instance_) {
    if (device_) {
      if (swap_chain_) {
        swap_chain_->Destroy();
        spdlog::debug("swap chain destroyed");
        delete swap_chain_;
      }
      if (pipeline_) {
        pipeline_->Destroy(device_->device_);
        spdlog::debug("pipeline destroyed");
        delete pipeline_;
      }
      if (render_pass_) {
        render_pass_->Destroy(device_->device_);
        spdlog::debug("render pass destroyed");
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
      vkDestroySurfaceKHR(instance_, surface_, nullptr);
      spdlog::debug("window surface destroyed");
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

void Engine::CleanUpSwapChain() {
  descriptors_->Destroy(device_->device_);
  for (auto buffer : global_ubs_) {
    buffer.Destroy(device_->device_);
  }
  for (auto framebuffer : framebuffers_) {
    framebuffer.Destroy(device_->device_);
  }
  vkFreeCommandBuffers(device_->device_, device_->command_pool_,
                       static_cast<uint32_t>(device_->command_buffers_.size()),
                       device_->command_buffers_.data());
  if (pipeline_) {
    pipeline_->Destroy(device_->device_);
  }
  if (render_pass_) {
    render_pass_->Destroy(device_->device_);
  }
  if (swap_chain_) {
    swap_chain_->Destroy();
  }
}

void Engine::RecreateSwapChain() {
  vkDeviceWaitIdle(device_->device_);
  CleanUpSwapChain();
  physical_device_->swap_chain_support_details_ =
      physical_device_->QuerySwapChainSupport(physical_device_->device_,
                                              surface_, false);
  {  // create swap chain
    if (swap_chain_->Init(device_, physical_device_, window_, surface_) !=
        VK_SUCCESS) {
      spdlog::error("swap chain creation failed");
      CleanUp();
      exit(1);
    }
  }

  {  // create render pass
    if (render_pass_->Init(device_->device_, swap_chain_->image_format_) !=
        VK_SUCCESS) {
      CleanUp();
      exit(1);
    }
  }

  {  // global ubo
    VkDeviceSize size = sizeof(GlobalUniformData);
    global_ubs_.resize(swap_chain_->images_.size());
    for (size_t i = 0; i < global_ubs_.size(); ++i) {
      if (global_ubs_[i].Allocate(device_, nullptr, size,
                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) !=
          VK_SUCCESS) {
        CleanUp();
        exit(1);
      };
    }
  }

  {  // descriptors
    if (descriptors_->Init(device_, 1, 0, 0, swap_chain_->images_.size()) !=
        VK_SUCCESS) {
      CleanUp();
      exit(1);
    }
    for(size_t i=0;i<swap_chain_->images_.size();++i) {
      VkDescriptorBufferInfo buffer_info{};
      buffer_info.buffer = global_ubs_[i].buffer_;
      buffer_info.offset = 0;
      buffer_info.range = sizeof(GlobalUniformData);
      VkWriteDescriptorSet descriptor_write{};
      descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptor_write.dstSet = descriptors_->sets_[i];
      descriptor_write.dstBinding = 0;
      descriptor_write.dstArrayElement = 0;
      descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptor_write.descriptorCount = 1;
      descriptor_write.pBufferInfo = &buffer_info;
      vkUpdateDescriptorSets(device_->device_, 1, &descriptor_write, 0, nullptr);
    }
    
  }

  {  // create pipeline
    if (pipeline_->Init(device_->device_, swap_chain_->extent_,
                        render_pass_->render_pass_,
                        descriptors_) != VK_SUCCESS) {
      spdlog::error("pipeline creation failed");
      CleanUp();
      exit(1);
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
  }

  // allocate and record command buffers
  device_->AllocateCommandBuffers(swap_chain_);
  {
    float aspect = 1.0;
    if (swap_chain_->extent_.height)
      aspect = float(swap_chain_->extent_.width) / swap_chain_->extent_.height;
    camera_->ResetAspect(aspect);
    camera_->UpdateData();
  }
}

void Engine::WindowResizeCallback(GLFWwindow* window, int width, int height) {
  auto engine = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
  engine->window_resized_ = true;
}

};  // namespace Rain