#!lua

workspace "TauIR"
    configurations {
        "Debug",
        "Release"
    }
    platforms { "Win64" }

    cdialect "C11"
    cppdialect "C++17"
    floatingpoint "Fast"
    floatingpointexceptions "off"
    rtti "off"
    clr "off"
    functionlevellinking "on"
    intrinsics "on"
    largeaddressaware "on"
    vectorextensions "AVX2"
    stringpooling "on"
    staticruntime "on"
    flags { "LinkTimeOptimization" }

    filter { "platforms:Win64" }
        system "Windows"
        architecture "x86_64"
        _arch = "x64"
        defines {
            "_CRT_SECURE_NO_WARNINGS",
            "_HAS_EXCEPTIONS=0"
        }
        characterset "MBCS"

    filter "configurations:Debug"
        optimize "Debug"
        inlining "Explicit"
        runtime "Debug"

    filter "configurations:Release"
        optimize "Speed"
        inlining "Auto"
        runtime "Release"

    filter {}

    targetdir "%{wks.location}/bin/%{cfg.shortname}-%{_arch}"

    project "TauIRLib"
        kind "StaticLib"
        language "C++"
        toolset "clang"
        location "TauIRLib"

        files { 
            "%{prj.location}/**.h", 
            "%{prj.location}/**.hpp", 
            "%{prj.location}/src/**.c", 
            "%{prj.location}/src/**.cpp" 
        }


        includedirs {
            "%{prj.location}/include",
            "%{wks.location}/libs/TauUtils/include"
        }

    project "TauIRTest"
        kind "ConsoleApp"
        language "C++"
        toolset "clang"
        location "TauIRTest"

        files { 
            "%{prj.location}/**.h", 
            "%{prj.location}/**.hpp", 
            "%{prj.location}/src/**.c", 
            "%{prj.location}/src/**.cpp" 
        }

        includedirs {
            "%{prj.location}/include",
            "%{wks.location}/libs/TauUtils/include",
            "%{wks.location}/TauIRLib/include"
        }

        libdirs {
            "%{cfg.outdir}",
            "%{wks.location}/libs/TauUtils/bin/%{cfg.shortname}-%{_arch}"
        }

        links {
            "TauUtils.lib",
            "TauIRLib.lib"
        }

