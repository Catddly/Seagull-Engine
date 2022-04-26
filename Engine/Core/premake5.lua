project "SCore"
    language "C++"
    cppdialect "C++17"

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
    kind "SharedLib"
    runtime "Debug"
    symbols "on"
    -- To use tracy, we have to make __LINE__ constexpr (__LINE__ is not a constexpr since C++17 or later in MSVC).
    -- Use "/Zi" for Debug Infomation Format in C++ Settings can make __LINE__ in C++17 constexpr.
    debugformat "Default"
    editandcontinue "Off"
    staticruntime "off"
    defines
    {
        "SG_BUILD_DLL"
    }
    -- enable if you want to build a dll
    postbuildcommands
    {
        ("{COPY} %{cfg.buildtarget.relpath} \"../../bin/" .. outputdir .. "/Sandbox/\"")
    }

filter "configurations:DebugStatic"
    kind "StaticLib"
    runtime "Debug"
    symbols "on"
    staticruntime "on"
    -- To use tracy, we have to make __LINE__ constexpr (__LINE__ is not a constexpr since C++17 or later in MSVC).
    -- Use "/Zi" for Debug Infomation Format in C++ Settings can make __LINE__ in C++17 constexpr.
    debugformat "Default"
    editandcontinue "Off"

filter "configurations:Release"
    kind "SharedLib"
    runtime "Release"
    optimize "Size"
    staticruntime "off"
    defines
    {
        "SG_BUILD_DLL",
    }
    postbuildcommands
    {
        ("{COPY} %{cfg.buildtarget.relpath} \"../../bin/" .. outputdir .. "/Sandbox/\"")
    }