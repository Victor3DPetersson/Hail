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
	group ("Utilities/Engine_ResourceHandling")
		include ("Engine_ResourceHandling")
	include ("ReflectionCodeGenerator")
group ("")
