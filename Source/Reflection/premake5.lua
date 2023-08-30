project "Reflection"
	location "%{dirs.localdir}"
	
	print ("Building Reflection...")
		
	language "C++"
	cppdialect "C++17"
	kind "StaticLib"

	targetdir ("%{dirs.libdir}")
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.intdir}")
	debugdir "%{dirs.outdir}"

	pchheader "Reflection_PCH.h"
	pchsource "Reflection_PCH.cpp"

	files {
		"%{dirs.srcdir}/Reflection/**.h",
		"%{dirs.srcdir}/Reflection/**.hpp",
		"%{dirs.srcdir}/Reflection/**.cpp",
	}

	includedirs {
		".",
		"%{dirs.srcdir}/Shared/",
	}

	libdirs { "%{dirs.libdir}" }	
	links { 
		"Shared"
		 }
