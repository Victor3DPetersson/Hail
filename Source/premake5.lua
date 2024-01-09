group ("Core")
	include ("Engine")
group ("")

group ("Game")
	include ("Game")
group ("")

group ("Launchers")
	include ("Launcher")
group ("")

group ("Utilities")
	include ("Shared")
	group ("Utilities/ResourceCompilation")
		include ("ResourceCompiler")
	include ("ReflectionCodeGenerator")
group ("")
