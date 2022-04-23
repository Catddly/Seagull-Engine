workspace "Seagull"
    architecture "x64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
		"Release"
    }

    platforms
    {
        "Win64",
    }

    flags
    {
		"MultiProcessorCompile"
    }

-- Debug-windows-x64
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
IncludeDir = { }

group "Seagull Engine"

    include "Engine/Core/"
    include "Engine/3DEngine/"
    include "Engine/RendererVulkan/"

group ""

group "Libs"

    include "Libs/volk/"
    include "Libs/mimalloc/"
    include "Libs/eastl/"
    include "Libs/assimp/"
    include "Libs/ktx/"
    include "Libs/imgui/"
    include "Libs/spirv-cross/"
    include "Libs/tracy/"

group ""

group "Tools"
group ""

group "Runtime"

    project "Sandbox"
        location "User/Sandbox"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        -- bin/Debug-windows-x64/Seagull Core
        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        -- bin-int/Debug-windows-x64/Seagull Core
        objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

        pchheader "StdAfx.h"
        pchsource "User/Sandbox/StdAfx.cpp"

        -- include files
        files
        {
            "User/Sandbox/**.h",
            "User/Sandbox/**.cpp"
        }

        includedirs
        {
            "Engine/",
            "Engine/Core/Public/",
            "Libs/",
            "Libs/eastl/include/",
            "Libs/glm/",
        }

        defines
        {
            -- "SG_BUILD_DLL",
        }

        links
        {
            "mimalloc",
            "eastl",
            "tracy_server",
            "SCore",
            "S3DEngine",
            "SRendererVulkan",
        }

    filter "system:windows"
        systemversion "latest"
        defines "SG_PLATFORM_WINDOWS"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        -- To use tracy, we have to make __LINE__ constexpr (__LINE__ is not a constexpr since C++17 or later in MSVC).
        -- Use "/Zi" for Debug Infomation Format in C++ Settings can make __LINE__ in C++17 constexpr.
        debugformat "Default"
        editandcontinue "Off"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter "platforms:Win64"
        system "Windows"
        architecture "x86_64"

group ""