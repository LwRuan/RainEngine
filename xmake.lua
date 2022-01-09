set_xmakever("2.5.9")

add_requires("glfw", "spdlog", "cmake::Vulkan")
add_rules("mode.release", "mode.debug")
set_languages("cxx17")

target("RainEngine")
    set_kind("binary")
    add_includedirs("src/engine", "src/renderer")
    add_files("src/main.cpp", "src/engine/*.cpp", "src/renderer/*/*.cpp")
    add_packages("glfw", "spdlog","cmake::Vulkan" , {public=true})
    set_targetdir("bin")