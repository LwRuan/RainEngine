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
    if (swap_chain_->Init(device_, physical_device_, window_, surface_) != VK_SUCCESS) {
      spdlog::error("swap chain creation failed");
      CleanUp();
      exit(1);
    } else {
      spdlog::debug("swap chain created");
    }
  }

  { // create pipeline
    pipeline_ = new Pipeline;
    if(pipeline_->Init(device_->device_) != VK_SUCCESS) {
      spdlog::error("pipeline creation failed");
      CleanUp();
      exit(1);
    } else {
      spdlog::debug("pipeline created");
    }
  }
}

void Engine::MainLoop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  }
}

void Engine::CleanUp() {
  if (instance_) {
    if (device_) {
      if(swap_chain_) {
        swap_chain_->Destroy();
        delete swap_chain_;
      }
      if(pipeline_) {
        pipeline_->Destroy(device_->device_);
        delete pipeline_;
      }
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