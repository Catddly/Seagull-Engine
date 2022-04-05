project "ktx"
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
        "source/**.c",
    }

    removefiles
    {
        "source/writer_v1.c",
        "source/glloader.c",
        "source/vkloader.c",
    }

    defines
    {
    }

    includedirs
    {
        "include/",
        "other_include/",
        "../", -- include vulkan
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