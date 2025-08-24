project "Launcher"
	location "%{dirs.srcdir}/Launcher"

	print ("Building Launcher...")
	
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"

	targetdir ("%{dirs.outdir}")
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.intdir}")

	pchheader "Launcher_PCH.h"
	pchsource "Launcher_PCH.cpp"

	debugdir ("%{dirs.outdir}") --needed?

	files { "**.h", "**.cpp", }

	includedirs {
		"%{dirs.srcdir}/Engine/",
		"%{dirs.srcdir}/Game/",
		"%{dirs.srcdir}/Shared/",
		"%{dirs.extdir}/dbghelp/",
	}

	libdirs { "%{dirs.libdir}" }

	links { 
		"Engine",
		"Game"
	}

filter { "platforms:Windows" }
	libdirs {
		"%{dirs.extdir}/dbghelp/"
	 }	
	links { 
		"dbghelp"
		 }