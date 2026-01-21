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
        "%{prj.name}/res/Shaders/**.hlsl",
        "%{prj.name}/vendor/ImGui/imgui.cpp",
        "%{prj.name}/vendor/ImGui/imgui.h",
        "%{prj.name}/vendor/ImGui/imconfig.h",
        "%{prj.name}/vendor/ImGui/imgui_demo.cpp",
        "%{prj.name}/vendor/ImGui/imgui_draw.cpp",
        "%{prj.name}/vendor/ImGui/imgui_internal.h",
        "%{prj.name}/vendor/ImGui/imgui_tables.cpp",
        "%{prj.name}/vendor/ImGui/imgui_widgets.cpp",
        "%{prj.name}/vendor/ImGui/imstb_rectpack.h",
        "%{prj.name}/vendor/ImGui/imstb_textedit.h",
        "%{prj.name}/vendor/ImGui/imstb_truetype.h",
        "%{prj.name}/vendor/ImGui/backends/imgui_impl_dx11.cpp",
        "%{prj.name}/vendor/ImGui/backends/imgui_impl_dx11.h",
        "%{prj.name}/vendor/ImGui/backends/imgui_impl_win32.cpp",
        "%{prj.name}/vendor/ImGui/backends/imgui_impl_win32.h",
        "%{prj.name}/vendor/ImGui/misc/cpp/**.cpp",
        "%{prj.name}/vendor/ImGui/misc/cpp/**.h"
    }

    includedirs
    {
        "%{prj.name}/vendor/ImGui",
        "%{prj.name}/vendor/Assimp",
        "%{prj.name}/vendor/Stb_Image"
    }

    links
    {
        "D3D11.lib",
        "D3DCompiler.lib",
        "User32.lib",
        "DXGI.lib",
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
        links { "%{wks.location}/RayTracingINZ/vendor/Assimp/lib/ReleaseAssimp/assimp-vc143-mt.lib" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        links { "%{wks.location}/RayTracingINZ/vendor/Assimp/lib/DebugAssimp/assimp-vc143-mtd.lib" }