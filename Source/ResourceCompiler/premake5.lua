project "ResourceCompiler"
	location "%{dirs.localdir}"

	print ("Building ResourceCompiler...")
	print (dirs.srcdir)
	print (dirs.extdir)

	language "C++"
	cppdialect "C++17"
	kind "StaticLib"

	targetdir ("%{dirs.libdir}")
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.intdir}")
	debugdir "%{dirs.shaderindir}"

	pchheader "ResourceCompiler_PCH.h"
	pchsource "ResourceCompiler_PCH.cpp"

	files {
		"%{dirs.srcdir}/ResourceCompiler/**.h",
		"%{dirs.srcdir}/ResourceCompiler/**.cpp",
	}

	includedirs {
		".",
		"%{dirs.srcdir}/Shared/",
		"%{dirs.extdir}/Vulkan/Include/",
	}
	defines {
	 	 	'SHADER_DIR_IN="' .. (dirs.shadersindir):gsub("%\\", "/") .. '/"',
	 	'SHADER_DIR_OUT="' .. (dirs.shadersoutdir):gsub("%\\", "/") .. '/"',
	 	'TEXTURES_DIR_IN="' .. (dirs.texturesindir):gsub("%\\", "/") .. '/"',
	 	'TEXTURES_DIR_OUT="' .. (dirs.texturesoutdir):gsub("%\\", "/") .. '/"'
	}
	libdirs { "%{dirs.libdir}", "%{dirs.extdir}/Vulkan/Lib/" }	
	links { 
		"Shared"
		 }
	filter { "configurations:Debug" }
	 	links { 
			"shaderc_combinedd"
			 }
	filter { "configurations:Release" }
		links { 
			"shaderc_combinedd"
			 }
	filter { "configurations:Production" }
		links { 
			"shaderc_combined"
			 }

