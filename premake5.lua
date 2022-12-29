require('vstudio')

workspace "SlaskLauncher"
	location ""
	startproject "Launcher"

	configurations {
		"Debug",
		"Release"
	}
	platforms { "Windows", "OSX" }

	defines { "symbols" }

filter { "configurations:Debug" }
  defines { "DEBUG" }
  symbols "On"

filter { "configurations:Release" }
  defines { "NDEBUG" }
  optimize "On"

filter { "platforms:Windows" }
  defines { "PLATFORM_WINDOWS" }

filter { "platforms:OSX" }
  defines { "PLATFORM_OSX" }

dirs = {}
dirs.outdir = os.realpath("Generated/bin")
dirs.intdir = os.realpath("Generated/int-bin")
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


