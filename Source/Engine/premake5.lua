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
		"%{dirs.srcdir}/ShaderCompiler/",
	}

	libdirs { "%{dirs.libdir}" }	
	links { 
		"dearimgui",
		"ShaderCompiler",
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
	
	defines {
	 	'SHADER_DIR_IN="' .. dirs.shaderindir:gsub("%\\", "/") .. '/"',
	 	'SHADER_DIR_OUT="' .. dirs.shaderoutdir:gsub("%\\", "/") .. '/"'
	}
	--defines {
	--	'RESOURCE_DIR="' .. resdir.engine:gsub("%\\", "/") .. '/"',
	--}