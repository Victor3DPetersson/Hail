require('vstudio')

workspace "HailLancher"
	location ""
	startproject "Launcher"
	architecture "x64"

	configurations {
		"Debug",
		"Release",
		"Production"
	}
	platforms { "Windows", "OSX" }

	defines { "symbols" }

filter { "configurations:Debug" }
  defines { "DEBUG" }
  symbols "On"

filter { "configurations:Release" }
  defines { "NDEBUG" }
  optimize "On"

filter { "configurations:Production" }
  defines {  "DEBUG" }
  symbols "On"
  optimize "On"

filter { "platforms:Windows" }
  defines { "PLATFORM_WINDOWS" }

filter { "platforms:OSX" }
  defines { "PLATFORM_OSX" }

dirs = {}
dirs.projectdir = os.realpath("")
dirs.outdir = os.realpath("Generated/bin")
dirs.resourcesoutdir = os.realpath("Generated/bin/resources")
dirs.resourcesindir = os.realpath("Source/Resources")
dirs.texturesoutdir = os.realpath("Generated/bin/resources/textures")
dirs.texturesindir = os.realpath("Source/Resources/Textures")
dirs.shadersoutdir = os.realpath("Generated/bin/resources/shaders")
dirs.shadersindir = os.realpath("Source/Resources/Shaders")
dirs.sourcedir = os.realpath("Source")
dirs.intdir = os.realpath("Generated/int-bin")
dirs.reflectiongenerationdir = os.realpath("Generated/int-bin/generatedCode")
dirs.libdir = os.realpath("Generated/lib")
dirs.localdir = os.realpath("Generated/local")

for i,v in pairs(dirs) do
	if not os.isdir(v) then
		os.mkdir(v)
		print ('created: ' .. v)
	end
end

dirs.srcdir = os.realpath("Source")
dirs.extdir = os.realpath("External")


include ("External")
include ("Source")


