workspace "Seagull"
    architecture "x64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
		"Release"
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

    include "Libs/eastl/"
    include "Libs/mimalloc/"
    include "Libs/assimp/"
    include "Libs/imgui/"

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
            "User/%{prj.name}/**.h",
            "User/%{prj.name}/**.cpp"
        }

        includedirs
        {
            "Engine/",
            "Engine/Core/Public/",
            "Libs/",
            "Libs/eastl/include/",
        }

        defines
        {
            "SG_BUILD_DLL",
        }

        links
        {
            "mimalloc",
            "eastl",
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

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

group ""