project "SCore"
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    -- bin/Debug-windows-x64/Seagull Core
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    -- bin-int/Debug-windows-x64/Seagull Core
    objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "StdAfx.h"
    pchsource "StdAfx.cpp"

    -- include files
    files
    {
        "**.h",
        "**.cpp"
    }

    defines
    {
        "SG_BUILD_DLL",
        "SG_MODULE",
    }

    includedirs
    {
        "../",
        "../../Libs/",
        "../../Libs/eastl/include/",
        "Public",
    }

    links
    {
        "mimalloc",
        "eastl"
    }

    filter "system:windows"
    systemversion "latest"
    defines "SG_PLATFORM_WINDOWS"

filter "configurations:Debug"
    runtime "Debug"
    symbols "on"
    -- enable if you want to build a dll
    postbuildcommands
    {
        ("{COPY} %{cfg.buildtarget.relpath} \"../../bin/" .. outputdir .. "/Sandbox/\"")
    }

filter "configurations:Release"
    runtime "Release"
    optimize "Size"
    postbuildcommands
    {
        ("{COPY} %{cfg.buildtarget.relpath} \"../../bin/" .. outputdir .. "/Sandbox/\"")
    }