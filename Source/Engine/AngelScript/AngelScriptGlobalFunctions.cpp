#include "Engine_PCH.h"
#include "AngelScriptGlobalFunctions.h"
#include "angelscript.h"
using namespace Hail;

void Hail::AngelScript::RegisterGlobalMessages(asIScriptEngine* pScriptEngine)
{

	// Register the function that we want the scripts to call 
	int r = pScriptEngine->RegisterGlobalFunction("void Print(const string &in)", asFUNCTION(Print), asCALL_CDECL);
	H_ASSERT(r >= 0, "Failed to register global function");
	r = pScriptEngine->RegisterGlobalFunction("void PrintError(const string &in)", asFUNCTION(PrintError), asCALL_CDECL);
	H_ASSERT(r >= 0, "Failed to register global function");
	r = pScriptEngine->RegisterGlobalFunction("void PrintWarning(const string &in)", asFUNCTION(PrintWarning), asCALL_CDECL);
	H_ASSERT(r >= 0, "Failed to register global function");
}

void AngelScript::Print(std::string& stringToPrint)
{
	H_DEBUGMESSAGE(stringToPrint);
}

void Hail::AngelScript::PrintError(std::string& stringToPrint)
{
	H_ERROR(stringToPrint);
}

void Hail::AngelScript::PrintWarning(std::string& stringToPrint)
{
	H_WARNING(stringToPrint);
}


