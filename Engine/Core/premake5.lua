project "SCore"
    --kind "SharedLib"
    kind "StaticLib"
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
        -- "SG_BUILD_DLL",
        "SG_MODULE",

        "_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING", -- hash<Vector3f>
    }

    includedirs
    {
        "../",
        "../../Libs/",
        "../../Libs/eastl/include/",
        "Public",
        "../../Libs/assimp/include/",
        "../../Libs/spirv-cross/include/",
        "../../Libs/glm/",
        "../../Libs/ktx/include/",
        "../../Libs/tracy/",
    }

    links
    {
        "mimalloc",
        "assimp",
        "eastl",
        "ktx",
        "spirv-cross",
        "tracy_server",
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