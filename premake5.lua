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

    include "Engine/Common/"
    include "Engine/3DEngine/"
    include "Engine/Core/"
    include "Engine/Utility/"
    -- include "Engine/Renderer/"
    include "Engine/RendererVulkan/"
    include "Engine/Editor/"
    include "Engine/Input/"

group ""

group "Libs"

    include "Libs/mimalloc/"
    include "Libs/mimalloc-static/"

group ""

group "Tools"
group ""

group "Runtime"

    project "Sandbox"
        location "Runtime/Sandbox"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        -- bin/Debug-windows-x64/Seagull Core
        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        -- bin-int/Debug-windows-x64/Seagull Core
        objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

        pchheader "StdAfx.h"
        pchsource "Runtime/Sandbox/StdAfx.cpp"

        -- include files
        files
        {
            "Runtime/%{prj.name}/**.h",
            "Runtime/%{prj.name}/**.cpp"
        }

        includedirs
        {
            "Engine/",
            "Libs/",
            "Libs/eastl/include/",
        }

        defines
        {
        }

        links
        {
            "mimalloc-static",
            "S3DEngine",
            "SRendererVulkan",
            "SCore",
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