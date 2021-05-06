workspace "Seagull"
    architecture "x64"
    startproject "Sandbox"

    configurations
    {
        "Debug", 
		"Release"
    }

    flags
    {
		"MultiProcessorCompile"
    }

-- Debug-windows-x64
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
IncludeDir = { }

-- copyfiledir = "\"$(SolutionDir)bin\\" .. outputdir .. "\\%{prj.name}\\$(ProjectName).dll\""
-- copylibdir  = "\"$(ProjectDir)bin\\%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}\\$(ProjectName)\\"
-- copydstdir  = "\"$(SolutionDir)bin\\" .. outputdir .. "\\Sandbox\\\""

group "Sandbox"

    project "Sandbox"
        location "User/Sandbox"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        -- bin/Debug-windows-x64/Seagull Core
        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        -- bin-int/Debug-windows-x64/Seagull Core
        objdir    ("bin-int/" .. outputdir .. "/%{prj.name}")

        -- pchheader "StdAfx.h"
        -- pchsource "Sandbox/Sandbox/StdAfx.cpp"

        -- include files
        files
        {
            "User/%{prj.name}/**.h",
            "User/%{prj.name}/**.cpp"
        }

        includedirs
        {
            "Engine/Common/"
        }

        links
        {
            "SCommon"
        }

    filter "system:windows"
        systemversion "latest"
        defines "SG_PLATFORM_WINDOWS"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

group ""

group "SeagullEngine"

    include "Engine/Common/"

group ""