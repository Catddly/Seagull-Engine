project "tracy_server"
    kind "StaticLib"
    language "C++"
    staticruntime "off"
    -- To use tracy, we have to make __LINE__ constexpr (__LINE__ is not a constexpr since C++17 or later in MSVC).
    -- Use "/Zi" for Debug Infomation Format in C++ Settings can make __LINE__ in C++17 constexpr.
    debugformat "Default"
    editandcontinue "Off"

    -- bin/Debug-windows-x64/Seagull Core
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    -- bin-int/Debug-windows-x64/Seagull Core
    objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

    -- include files
    files
    {
        "../tracy/TracyClient.cpp"
    }

    defines
    {
        "TRACY_ENABLE",
    }

    includedirs
    {
    }

    links
    {
    }

filter "configurations:Debug"
    runtime "Debug"
    symbols "on"
    -- enable if you want to build a dll
    -- postbuildcommands
    -- {
    --     ("{COPY} %{cfg.buildtarget.relpath} \"../../bin/" .. outputdir .. "/Sandbox/\""),
    --     ("{COPY} \"libs/mimalloc-redirect.dll\" \"../../bin/" .. outputdir .. "/Sandbox/\""),
    -- }

filter "configurations:Release"
    runtime "Release"
    optimize "on"
    -- postbuildcommands
    -- {
    --     ("{COPY} %{cfg.buildtarget.relpath} \"../../bin/" .. outputdir .. "/Sandbox/\""),
    --     ("{COPY} \"libs/mimalloc-redirect.dll\" \"../../bin/" .. outputdir .. "/Sandbox/\""),
    -- }