#include "Engine_PCH.h"
#include "AngelScriptErrorHandler.h"
#include "AngelScriptDebugger.h"

#include "angelscript.h"

using namespace Hail;

void AngelScript::ErrorHandler::SetScriptEngine(asIScriptEngine* pScriptEngine)
{
	// Set the message callback to receive information on errors in human readable form.
	int r = pScriptEngine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, asCALL_CDECL);
	H_ASSERT(r >= 0, "Failed to set AngelScript message callback");
}

void AngelScript::MessageCallback(const asSMessageInfo* msg, void* param)
{
	StringL message = StringL::Format("%s (%d, %d) : %s\n", msg->section, msg->row, msg->col, msg->message);
	if (msg->type == asMSGTYPE_WARNING)
	{
		H_WARNING(message);
	}
	else if (msg->type == asMSGTYPE_INFORMATION)
	{
		H_DEBUGMESSAGE(message);
	}
	else if (msg->type == asMSGTYPE_ERROR)
	{
		H_ERROR(message);
	}
}
