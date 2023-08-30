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
		prebuildcommands { "start %{dirs.outdir}/ReflectionCodeGenerator_%{cfg.buildcfg}.exe Game" }

		
