project "Engine"
	location "%{dirs.localdir}"
	
	print ("Building Engine...")
		
	language "C++"
	cppdialect "C++17"
	kind "StaticLib"

	targetdir ("%{dirs.libdir}")
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.intdir}")
	debugdir "%{dirs.outdir}"

	pchheader "Engine_PCH.h"
	pchsource "Engine_PCH.cpp"

	files {
		"%{dirs.srcdir}/Engine/**.h",
		"%{dirs.srcdir}/Engine/**.cpp",
	}

	includedirs {
		".",
		"%{dirs.extdir}/dearimgui/",
		"%{dirs.srcdir}/Shared/",
		"%{dirs.srcdir}/ResourceCompiler/",
	}

	libdirs { "%{dirs.libdir}" }	
	links { 
		"dearimgui",
		"ResourceCompiler",
		"Shared"
		 }
filter { "platforms:Windows" }
	libdirs {"%{dirs.extdir}/Vulkan/Lib/" }	
	links { 
		"vulkan-1"
		 }
 	includedirs {
		"%{dirs.extdir}/Vulkan/Include/",
	}
	
	dependson { "ReflectionCodeGenerator" }

	defines {
		'PROJECT_DIR="' .. (dirs.projectdir):gsub("%\\", "/") .. '/"',
	 	'SOURCE_DIR="' .. (dirs.sourcedir):gsub("%\\", "/") .. '/"',
	 	'RESOURCE_DIR="' .. (dirs.resourcesindir):gsub("%\\", "/") .. '/"',
	 	'RESOURCE_DIR_OUT="' .. (dirs.resourcesoutdir):gsub("%\\", "/") .. '/"',
	 	'SHADER_DIR_IN="' .. (dirs.shadersindir):gsub("%\\", "/") .. '/"',
	 	'SHADER_DIR_OUT="' .. (dirs.shadersoutdir):gsub("%\\", "/") .. '/"',
	 	'TEXTURES_DIR_IN="' .. (dirs.texturesindir):gsub("%\\", "/") .. '/"',
	 	'TEXTURES_DIR_OUT="' .. (dirs.texturesoutdir):gsub("%\\", "/") .. '/"'
	}

	filter { "system:windows" }
		prebuildcommands { "start %{dirs.outdir}/ReflectionCodeGenerator_%{cfg.buildcfg}.exe Engine" }

	--filter { "system:windows" } Add later once I fix the parser for longer lines than 256
	--	prebuildcommands { "start %{dirs.outdir}/ReflectionCodeGenerator_%{cfg.buildcfg}.exe Engine" }

	--defines {
	--	'RESOURCE_DIR="' .. resdir.engine:gsub("%\\", "/") .. '/"',
	--}

