project "Engine"
	location "%{dirs.localdir}"
	
	print ("Building Engine...")
	pathy = dirs.extdir .. "/Vulkan/Lib/"
	print (pathy)
	print (dirs.localdir)
		
	language "C++"
	cppdialect "C++17"
	kind "StaticLib"

	targetdir ("%{dirs.libdir}")
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.intdir}")

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
	}

	libdirs { "%{dirs.libdir}" }	
	links { 
		"dearimgui",
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

	--defines {
	--	'RESOURCE_DIR="' .. resdir.engine:gsub("%\\", "/") .. '/"',
	--}