project "ShaderCompiler"
	location "%{dirs.localdir}"

	print ("Building ShaderCompiler...")
	print (dirs.srcdir)
	print (dirs.extdir)

	language "C++"
	cppdialect "C++17"
	kind "StaticLib"

	targetdir ("%{dirs.libdir}")
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.intdir}")
	debugdir "%{dirs.shaderindir}"

	pchheader "ShaderCompiler_PCH.h"
	pchsource "ShaderCompiler_PCH.cpp"

	files {
		"%{dirs.srcdir}/ShaderCompiler/**.h",
		"%{dirs.srcdir}/ShaderCompiler/**.cpp",
	}

	includedirs {
		".",
		"%{dirs.srcdir}/Shared/",
		"%{dirs.extdir}/Vulkan/Include/",
	}
	defines {
	 	'SHADER_DIR_IN="' .. dirs.shaderindir:gsub("%\\", "/") .. '/"',
	 	'SHADER_DIR_OUT="' .. dirs.shaderoutdir:gsub("%\\", "/") .. '/"'
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

