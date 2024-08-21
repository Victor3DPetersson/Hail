#include "ReflectionCodeGenerator_PCH.h"
#include "CppParser.h"
#include "Utility\StringUtility.h"
#include "DebugMacros.h"
int main(int argc, char* argv[])
{
	Debug_PrintConsoleStringL(Hail::StringL::Format("You have entered %d arguments:\n", argc));

	for (int i = 0; i < argc; i++) {
		Debug_PrintConsoleStringL(Hail::StringL::Format("%s\n", argv[i]));
		if (Hail::StringCompare(argv[i], "Engine") || Hail::StringCompare(argv[i], "Game"))
		{
			Hail::ParseAndGenerateCodeForProjects(argv[i]);
		}
	}
	return 0;
}