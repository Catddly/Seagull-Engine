project "eastl"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    -- bin/Debug-windows-x64/Seagull Core
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    -- bin-int/Debug-windows-x64/Seagull Core
    objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

    -- include files
    files
    {
        "source/**.cpp"
    }

    defines
    {
    }

    includedirs
    {
        "include/",
        "../../Engine/",
        "../../Engine/Core/",
        "../../Engine/Core/Public/",
    }

    filter "system:windows"
    systemversion "latest"
    defines "SG_PLATFORM_WINDOWS"

filter "configurations:Debug"
    runtime "Debug"
    symbols "on"
    -- enable if you want to build a dll

filter "configurations:Release"
    runtime "Release"
    optimize "on"