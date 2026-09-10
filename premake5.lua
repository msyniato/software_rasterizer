workspace "software_renderer"
	configurations { "Debug", "Release", "RelWithDebInfo" }
	architecture "x64"
	location "build"
	cppdialect "c++20"
	startproject "software_renderer"

	filter "configurations:Debug"
	defines { "DEBUG" }
	symbols "On"
	runtime "Debug"

	filter "configurations:Release"
	defines { "NDEBUG" }
	optimize "On"
	runtime "Release"

	filter "configurations:RelWithDebInfo"
	defines { "NDEBUG" }
	optimize "On"
	runtime "Release"
	symbols "On"
	filter {}

project "software_renderer"
	
	kind "ConsoleApp"  -- Change to "WindowedApp" to remove console access

	language "C++"
	
	targetdir "%{_MAIN_SCRIPT_DIR}/build/%{prj.name}/%{cfg.buildcfg}"
	debugdir "%{cfg.targetdir}"
	
	includedirs {
		"./vendor/SDL3/include",
		"./include/"
	}

	libdirs {
    		"vendor/SDL3/lib/x64"
	}

	links {
    		"SDL3"
	}
	
	files {
		"./include/**.hpp",
		"./_src/**.cpp",
		"./main.cpp"
	}
	
	postbuildcommands {
        	'{COPYFILE} "%{_MAIN_SCRIPT_DIR}/vendor/SDL3/lib/x64/SDL3.dll" "%{cfg.targetdir}/SDL3.dll"'
    	}