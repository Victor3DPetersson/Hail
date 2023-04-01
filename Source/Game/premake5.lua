project "Game"
	location "%{dirs.localdir}"

	print ("Building Game...")
		
	language "C++"
	cppdialect "C++17"
	kind "StaticLib"

	targetdir ("%{dirs.libdir}")
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.intdir}")

	pchheader "Game_PCH.h"
	pchsource "Game_PCH.cpp"

	debugdir ("%{dirs.outdir}")

	files {
		"%{dirs.srcdir}/Game/**.h",
		"%{dirs.srcdir}/Game/**.cpp",
	}


	includedirs {
		".",
		"./**",
		"%{dirs.srcdir}/Shared/",
	}


	libdirs { "%{dirs.libdir}" }	
	links { 
		"Engine",
		"Shared"
		 }