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
        "%{prj.name}/src/**.h",
        "%{prj.name}/res/Shaders/**.hlsl"
    }

    links
    {
        "D3D11.lib",
        "D3DCompiler.lib",
        "User32.lib",
        "DXGI.lib"
    }

    filter "files:**PixelShader.hlsl"
        buildmessage "Compiling PixelShader"
        buildcommands {
            "fxc /T ps_5_0 /Fo %{wks.location}/RayTracingINZ/res/shaders/PixelShader.fxc %{file.relpath}"
        }
        buildoutputs {"%{wks.location}/RayTracingINZ/res/shaders/PixelShader.fxc"}

    filter "files:**VertexShader.hlsl"
        buildmessage "Compiling VertexShader"
        buildcommands {
            "fxc /T vs_5_0 /Fo %{wks.location}/RayTracingINZ/res/shaders/VertexShader.fxc %{file.relpath}"
        }
        buildoutputs {"%{wks.location}/RayTracingINZ/res/shaders/VertexShader.fxc"}

    filter "files:**ComputeShader.hlsl"
        buildmessage "Compiling ComputeShader"
        buildcommands {
            "fxc /T cs_5_0 /Fo %{wks.location}/RayTracingINZ/res/shaders/ComputeShader.fxc %{file.relpath}"
        }
        buildoutputs {"%{wks.location}/RayTracingINZ/res/shaders/ComputeShader.fxc"}

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