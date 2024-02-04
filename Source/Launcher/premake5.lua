project "Launcher"
	location "%{dirs.srcdir}/Launcher"

	print ("Building Launcher...")
	
	kind "ConsoleApp"
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
	}

	libdirs { "%{dirs.libdir}" }

	links { 
		"Engine",
		"Game"
	}
