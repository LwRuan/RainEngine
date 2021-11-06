set_xmakever("2.5.6")

add_requires("glfw")

target("RainEngine")
    set_kind("binary")
    add_files("src/main.cpp")
    add_packages("glfw")
    set_targetdir("bin")
    on_load(function (target)
        target:add(find_packages("cmake::Vulkan"))
    end)