project "SCommon"
    kind "Utility"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    -- bin/Debug-windows-x64/Seagull Core
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    -- bin-int/Debug-windows-x64/Seagull Core
    objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "StdAfx.h"
    pchsource "StdAfx.cpp"

    files
    {
        "**.h"
    }

    defines
    {
    }

    includedirs
    {
        "../",
    }

    links
    {
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