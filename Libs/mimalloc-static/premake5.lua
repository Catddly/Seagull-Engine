project "mimalloc-static"
    kind "StaticLib"
    language "C"
    staticruntime "off"

    -- bin/Debug-windows-x64/Seagull Core
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    -- bin-int/Debug-windows-x64/Seagull Core
    objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

    -- include files
    files
    {
        "../mimalloc/source/alloc.c",
        "../mimalloc/source/alloc-aligned.c",
        "../mimalloc/source/alloc-posix.c",
        "../mimalloc/source/arena.c",
        "../mimalloc/source/bitmap.c",
        "../mimalloc/source/heap.c",
        "../mimalloc/source/init.c",
        "../mimalloc/source/options.c",
        "../mimalloc/source/os.c",
        "../mimalloc/source/page.c",
        "../mimalloc/source/random.c",
        "../mimalloc/source/region.c",
        "../mimalloc/source/segment.c",
        "../mimalloc/source/stats.c",
    }

    defines
    {
        "WIN32",
        "_WINDOWS",
        "MI_STATIC_LIB",
    }

    includedirs
    {
        "../mimalloc/include",
    }

    links
    {
    }

filter "configurations:Debug"
    runtime "Debug"
    symbols "on"
    -- enable if you want to build a dll

filter "configurations:Release"
    runtime "Release"
    optimize "on"