set_xmakever("2.5.9")

add_requires("glfw", "spdlog", "cmake::Vulkan")
add_rules("mode.release", "mode.debug")
set_languages("cxx17")

target("engine")
    set_kind("static")
    add_includedirs("src/engine", "src")
    add_files("src/engine/*.cpp", "src/device/*.cpp", "src/vkext/*.cpp", "src/surface/*.cpp")
    add_packages("glfw", "spdlog","cmake::Vulkan" , {public=true})

target("RainEngine")
    set_kind("binary")
    add_includedirs("src/engine", "src")
    add_files("src/main.cpp")
    add_deps("engine")
    add_packages("cmake::Vulkan")
    set_targetdir("bin")