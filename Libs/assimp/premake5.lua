project "assimp"
    kind "StaticLib"
    language "C++"
    staticruntime "off"

    -- bin/Debug-windows-x64/Seagull Core
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    -- bin-int/Debug-windows-x64/Seagull Core
    objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

    -- including files
    files
    {
        "code/AssetLib/**.cpp",
        "code/CApi/**.cpp",
        "code/Common/**.cpp",
        "code/Material/**.cpp",
        "code/Pbrt/**.cpp",
        "code/PostProcessing/**.cpp",

        -- contrib
        "contrib/Open3DGC/**.cpp",
        "contrib/openddlparser/code/**.cpp",

        "contrib/zip/src/**.c",
        "contrib/unzip/**.c",
        "contrib/zlib/**.c",

        "contrib/poly2tri/poly2tri/**.cc",
        "contrib/clipper/**.cpp",
    }
    
    -- excluding files
    removefiles 
    { 
        "code/AssetLib/IFC/IFCReaderGen_4.cpp", -- missing AssimpPCH.h

        "contrib/zlib/contrib/**.cpp",
        "contrib/zlib/contrib/**.c",
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "_SCL_SECURE_NO_WARNINGS",
        "OPENDDL_STATIC_LIBARY",
        "WIN32_LEAN_AND_MEAN",
        "ASSIMP_BUILD_NO_C4D_IMPORTER",
        "OPENDDLPARSER_BUILD",
        "ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC=1",
        "RAPIDJSON_HAS_STDSTRING=1",
        "RAPIDJSON_NOMEMBERITERATORCLASS",
        "ASSIMP_BUILD_NO_M3D_IMPORTER",
        "ASSIMP_BUILD_NO_M3D_EXPORTER",
    }

    includedirs
    {
        "include/",
        "contrib/",
        "code/",
        "contrib/pugixml/src/", -- include <pugixml.hpp>
        "../assimp/",     -- include <contrib/..>
        "contrib/unzip/", -- include <unzip.h> in ZipArchiveIOSystem.cpp
        "contrib/rapidjson/include/", -- include <rapidjson/document.h ..> in glTF2Asset.h
        "contrib/zlib/", -- include <zlib.h> in unzip.h
        "contrib/openddlparser/include/", -- include <openddlparser/OpenDDLParser.h> in OpenGEXImporter.cpp
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