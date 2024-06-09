project "Engine_ResourceHandling"
	location "%{dirs.srcdir}/Engine_ResourceHandling"

	print ("Building Engine_ResourceHandling...")
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
		"%{dirs.srcdir}/Engine_ResourceHandling/**.h",
		"%{dirs.srcdir}/Engine_ResourceHandling/**.cpp",
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
			"shaderc_combinedd",
			"spirv-cross-cored",
			"spirv-cross-reflectd",
			"spirv-cross-glsld"
			 }
	filter { "configurations:Release" }
		links { 
			"shaderc_combined",
			"spirv-cross-core",
			"spirv-cross-reflect",
			"spirv-cross-glsl"
			 }
	filter { "configurations:Production" }
		links { 
			"shaderc_combined",
			"spirv-cross-core",
			"spirv-cross-reflect",
			"spirv-cross-glsl"
			 }

