project "SRendererVulkan"
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
    }

    includedirs
    {
        "../",
        "../Core/Public/",
        "../../Libs/",
        "../../Libs/eastl/include/", -- eastl
        "../../Libs/imgui/include/", -- imgui
        "../../Libs/volk/include/", -- volk
        "../../Libs/volk/source/",
        "../../Libs/glm/",  -- glm
        "../../Libs/ktx/include/",
        "../../Libs/vulkan_memory_allocator/",
    }

    links
    {
        "volk",
        "mimalloc",
        "eastl",
        "imgui",
        "ktx",
        "tracy_server",
        "SCore",
    }

    ignoredefaultlibraries 
    { 
        "MSVCRT",
        "LIBCMTD"
    }

    filter "system:windows"
    systemversion "latest"
    defines "SG_PLATFORM_WINDOWS"
    
    filter { "platforms:Win64" }
        defines { "VK_USE_PLATFORM_WIN32_KHR" }

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
        "SG_BUILD_DLL",
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