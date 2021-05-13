project "mimalloc"
    kind "SharedLib"
    language "C"
    staticruntime "off"

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
        "source/bitmap.h",
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
        "MI_SHARED_LIB",
        "MI_SHARED_LIB_EXPORT",
    }

    includedirs
    {
        "include",
    }

    links
    {
        "libs/mimalloc-redirect",
    }

filter "configurations:Debug"
    runtime "Debug"
    symbols "on"
    -- enable if you want to build a dll
    postbuildcommands
    {
        ("{COPY} %{cfg.buildtarget.relpath} \"../../../bin/" .. outputdir .. "/Sandbox/\""),
        ("{COPY} \"libs/mimalloc-redirect.dll\" \"../../../bin/" .. outputdir .. "/Sandbox/\""),
    }

filter "configurations:Release"
    runtime "Release"
    optimize "on"
    postbuildcommands
    {
        ("{COPY} %{cfg.buildtarget.relpath} \"../../../bin/" .. outputdir .. "/Sandbox/\""),
        ("{COPY} \"libs/mimalloc-redirect.dll\" \"../../../bin/" .. outputdir .. "/Sandbox/\""),
    }