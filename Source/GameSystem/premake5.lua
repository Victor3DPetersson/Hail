project "GameSystem"
	location "%{dirs.localdir}"

	print ("Building GameSystem...")
		
	language "C++"
	cppdialect "C++17"
	kind "StaticLib"

	targetdir ("%{dirs.libdir}")
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.intdir}")

	pchheader "GameSystem_PCH.h"
	pchsource "GameSystem_PCH.cpp"

	debugdir ("%{dirs.outdir}")

	files {
		"%{dirs.srcdir}/GameSystem/**.h",
		"%{dirs.srcdir}/GameSystem/**.cpp",
	}


	includedirs {
		".",
		"./**",
		"%{dirs.srcdir}/Shared/"
	}

	dependson { "ReflectionCodeGenerator" }

	libdirs { "%{dirs.libdir}" }	
	links { 
		"Engine",
		"Reflection",
		"Shared"
		 }

		 	defines {

	 	'SOURCE_DIR="' .. (dirs.sourcedir):gsub("%\\", "/") .. '/"',
	 	'RESOURCE_DIR="' .. (dirs.resourcesindir):gsub("%\\", "/") .. '/"',
	 	'SHADER_DIR_IN="' .. (dirs.shadersindir):gsub("%\\", "/") .. '/"',
	 	'SHADER_DIR_OUT="' .. (dirs.shadersoutdir):gsub("%\\", "/") .. '/"',
	 	'TEXTURES_DIR_IN="' .. (dirs.texturesindir):gsub("%\\", "/") .. '/"',
	 	'TEXTURES_DIR_OUT="' .. (dirs.texturesoutdir):gsub("%\\", "/") .. '/"'
	}

	filter { "system:windows" }
		prebuildcommands { "start %{dirs.outdir}/ReflectionCodeGenerator_%{cfg.buildcfg}.exe GameSystem" }

		
