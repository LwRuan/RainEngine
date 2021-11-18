#include "app.h"

#include <cstring>

#include "spdlog/spdlog.h"

namespace Rain {
App::App() {
#ifdef NDEBUG
  enable_validation_layers_ = false;
#else
  enable_validation_layers_ = true;
#endif
}

void App::Init() {
  {  // init window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width_, height_, "Vulkan", nullptr, nullptr);
  }

  if (enable_validation_layers_) {
    if (!CheckValidationLayerSupport()) {
      spdlog::error("validation layers requested, but not available");
      exit(1);
    }
    debug_utils_ext_ = new DebugUtilsEXT();
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
      debug_utils_ext_->GetCreateInfo(debug_create_info);
      create_info.pNext =
          (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
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
}

void App::MainLoop() {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
}

void App::CleanUp() {
  if (instance_) {
    if (debug_utils_ext_) {
      debug_utils_ext_->Destroy(instance_);
      delete debug_utils_ext_;
    }
    vkDestroyInstance(instance_, nullptr);
    spdlog::debug("instance destroyed");
  }
  glfwDestroyWindow(window);
  glfwTerminate();
}

bool App::CheckValidationLayerSupport() {
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

std::vector<const char*> App::GetRequiredExtensions() {
  // functional extensions
  std::vector<const char*> extensions(extensions_);
  // debug extensions
  if (enable_validation_layers_) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  return extensions;
}

};  // namespace Rain