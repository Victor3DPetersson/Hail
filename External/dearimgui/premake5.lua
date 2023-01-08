project "dearimgui"
	location "%{dirs.localdir}"
		
	language "C++"
	cppdialect "C++17"
	kind "StaticLib"

	targetdir ("%{dirs.libdir}")
	targetname("%{prj.name}_%{cfg.buildcfg}")
	objdir ("%{dirs.intdir}")

	imguidir = dirs.extdir .. "/dearimgui"
	files {
		"%{imguidir}/*.h",
		"%{imguidir}/*.cpp",
		"%{imguidir}/backends/imgui_impl_win32.h",
		"%{imguidir}/backends/imgui_impl_win32.cpp",
		"%{imguidir}/backends/imgui_impl_vulkan.h",
		"%{imguidir}/backends/imgui_impl_vulkan.cpp",
		--"%{imguidir}/backends/imgui_impl_win32.h",
		--"%{imguidir}/backends/imgui_impl_win32.cpp",
		--"%{imguidir}/backends/imgui_impl_dx11.h",
		--"%{imguidir}/backends/imgui_impl_dx11.cpp",
	}

filter { "platforms:Windows" }
	includedirs {
		"%{dirs.extdir}/Vulkan/Include/",
	}


	includedirs { ".", }