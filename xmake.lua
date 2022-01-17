set_xmakever("2.5.9")

add_requires("glfw", "spdlog", "eigen", "cmake::Vulkan", "tinyobjloader")
add_rules("mode.release", "mode.debug")
set_languages("cxx17")

target("RainEngine")
    set_kind("binary")
    add_includedirs("src/engine", "src/common", "src/geometry", "src/physics", "src/renderer", "ext/imgui")
    add_files("src/main.cpp", "src/*/*.cpp", "src/*/*/*.cpp", "ext/imgui/*.cpp", "ext/imgui/backends/*.cpp")
    add_packages("glfw", "spdlog", "eigen", "cmake::Vulkan", "tinyobjloader", {public=true})
    set_targetdir("bin")