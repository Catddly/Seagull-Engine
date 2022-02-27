project "SRendererVulkan"
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
        "../Core/Public/",
        "../../Libs/",
        "../../Libs/eastl/include/", -- eastl
        "../../Libs/imgui/include/", -- imgui
        "../../Libs/volk/include/", -- volk
        "../../Libs/volk/source/"
    }

    links
    {
        "volk",
        "mimalloc",
        "eastl",
        "imgui",
        "SCore",
    }

    ignoredefaultlibraries 
    { 
        "LIBCMTD"
    }

    filter "system:windows"
    systemversion "latest"
    defines "SG_PLATFORM_WINDOWS"
    
    filter { "platforms:Win64" }
        defines { "VK_USE_PLATFORM_WIN32_KHR" }

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