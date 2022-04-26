project "mimalloc"
    kind "StaticLib"
    language "C"

    -- bin/Debug-windows-x64/Seagull Core
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    -- bin-int/Debug-windows-x64/Seagull Core
    objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

    -- include files
    files
    {
        "source/alloc.c",
        "source/alloc-aligned.c",
        "source/alloc-posix.c",
        "source/arena.c",
        "source/bitmap.c",
        "source/heap.c",
        "source/init.c",
        "source/options.c",
        "source/os.c",
        "source/page.c",
        "source/random.c",
        "source/region.c",
        "source/segment.c",
        "source/stats.c",
    }

    defines
    {
        "WIN32",
        "_WINDOWS",
        "MI_STATIC_LIB",
    }

    includedirs
    {
        "include",
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