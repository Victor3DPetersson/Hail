project "Engine"
	location "%{dirs.localdir}"
	
	print ("Building Engine...")
		
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
		"./**",
		"%{dirs.extdir}/dearimgui/",
		"%{dirs.srcdir}/Shared/",
	}

	libdirs { "%{dirs.libdir}" }	
	links { 
		"dearimgui",
		"Shared"
		 }

	--defines {
	--	'RESOURCE_DIR="' .. resdir.engine:gsub("%\\", "/") .. '/"',
	--}