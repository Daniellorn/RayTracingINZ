workspace "PracaINZ"
    configurations { "Debug", "Release" }
    architecture "x64"

output_dir = "%{cfg.architecture}-%{cfg.buildcfg}"

project "RayTracingINZ"
    location "RayTracingINZ"
    kind "WindowedApp"
    language "C++"

    targetdir ("bin/" .. output_dir)
    objdir ("bin/intermediates/" .. output_dir)

    files
    {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }

    links
    {
        "D3D11.lib",
        "D3DCompiler.lib",
        "User32.lib",
        "DXGI.lib"
    }

    filter "system:windows"
        cppdialect "C++23"
        staticruntime "On"
        systemversion "latest"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"