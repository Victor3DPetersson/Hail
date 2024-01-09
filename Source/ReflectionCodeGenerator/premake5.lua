project "ReflectionCodeGenerator"
	location "%{dirs.localdir}"
	
	print ("Building ReflectionCodeGenerator...")
		
	language "C++"
	cppdialect "C++17"
	kind "ConsoleApp"

	targetdir ("%{dirs.outdir}")
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.intdir}")
	debugdir "%{dirs.outdir}"

	pchheader "ReflectionCodeGenerator_PCH.h"
	pchsource "ReflectionCodeGenerator_PCH.cpp"

	files {
		"%{dirs.srcdir}/ReflectionCodeGenerator/**.h",
		"%{dirs.srcdir}/ReflectionCodeGenerator/**.cpp",
	}

	includedirs {
		".",
		"%{dirs.srcdir}/Shared/",
	}

	libdirs { "%{dirs.libdir}" }	
	links { 
		"Shared",
		 }
	
	defines {

	 	'SOURCE_DIR="' .. (dirs.sourcedir):gsub("%\\", "/") .. '"',
	}