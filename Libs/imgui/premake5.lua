project "imgui"
    kind "StaticLib"
    language "C++"

    -- bin/Debug-windows-x64/Seagull Core
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    -- bin-int/Debug-windows-x64/Seagull Core
    objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

    -- include files
    files
    {
        "source/**.cpp",
        -- "backends/**.cpp",
    }

    defines
    {
    }

    includedirs
    {
        "include/imgui/",
    }

    links
    {
    }

filter "configurations:Debug"
    runtime "Debug"
    symbols "on"
    staticruntime "off"
    -- enable if you want to build a dll
    -- postbuildcommands
    -- {
    --     ("{COPY} %{cfg.buildtarget.relpath} \"../../bin/" .. outputdir .. "/Sandbox/\""),
    --     ("{COPY} \"libs/mimalloc-redirect.dll\" \"../../bin/" .. outputdir .. "/Sandbox/\""),
    -- }

filter "configurations:DebugStatic"
    runtime "Debug"
    symbols "on"
    staticruntime "on"

filter "configurations:Release"
    runtime "Release"
    optimize "on"
    staticruntime "off"
    -- postbuildcommands
    -- {
    --     ("{COPY} %{cfg.buildtarget.relpath} \"../../bin/" .. outputdir .. "/Sandbox/\""),
    --     ("{COPY} \"libs/mimalloc-redirect.dll\" \"../../bin/" .. outputdir .. "/Sandbox/\""),
    -- }