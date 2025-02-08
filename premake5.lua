workspace "magic_memory"
    architecture "x64"
    startproject "examples"
    configurations { "debug", "release", "dist" }
    flags { "MultiProcessorCompile" }

    filter "system:windows"
        systemversion "latest"
        buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus", "/utf-8" }
        defines { "MC_PLATFORM_WINDOWS" }

    filter "configurations:debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"

    OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

    IncludeDir = {}
    IncludeDir["magic_memory"] = "%{wks.location}/magic_memory"
    IncludeDir["doctest"] = "%{wks.location}/vendors/doctest/doctest"

    include "magic_memory/premake5.lua"

    include "examples/premake5.lua"

    include "tests/premake5.lua"