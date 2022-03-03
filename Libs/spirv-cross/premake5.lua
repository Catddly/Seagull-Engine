project "spirv-cross"
    kind "StaticLib"
    language "C++"
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
        "SPIRV_CROSS_C_API_GLSL",
    }

    includedirs
    {
        "include/spirv-cross/",
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