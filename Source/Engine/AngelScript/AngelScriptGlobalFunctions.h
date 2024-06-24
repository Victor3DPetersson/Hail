#pragma once

class asIScriptEngine;
#include <string>
namespace Hail
{
	namespace AngelScript
	{
		void RegisterGlobalMessages(asIScriptEngine* pScriptEngine);

		//TODO: replace with cusotm string class
		void Print(std::string& stringToPrint);
		void PrintError(std::string& stringToPrint);
		void PrintWarning(std::string& stringToPrint);

	}
}