project "Shared"
	location "%{dirs.localdir}"
	
	print ("Building Shared...")
		
	language "C++"
	cppdialect "C++17"
	kind "StaticLib"

	targetdir ("%{dirs.libdir}")
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.intdir}")

	pchheader "Shared_PCH.h"
	pchsource "Shared_PCH.cpp"

	files {
		"%{dirs.srcdir}/Shared/**.h",
		"%{dirs.srcdir}/Shared/**.hpp",
		"%{dirs.srcdir}/Shared/**.cpp",
	}

	libdirs { "%{dirs.libdir}" }	

