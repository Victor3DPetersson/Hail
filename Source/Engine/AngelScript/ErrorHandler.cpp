#include "Engine_PCH.h"
#include "ErrorHandler.h"
#include "Debugger.h"
#include "Runner.h"

#include "angelscript.h"

using namespace Hail;

void AngelScript::ErrorHandler::SetScriptEngine(asIScriptEngine* pScriptEngine)
{
	// Set the message callback to receive information on errors in human readable form.
	int r = pScriptEngine->SetMessageCallback(asMETHOD(ErrorHandler, MessageCallback), this, asCALL_THISCALL);
	H_ASSERT(r >= 0, "Failed to set AngelScript message callback");
}

void AngelScript::ErrorHandler::MessageCallback(const asSMessageInfo* msg, void* param)
{
	StringL message = StringL::Format("%s (%d, %d) : %s\n", msg->section, msg->row, msg->col, msg->message);

	if (m_pRunner && m_pRunner->GetDebuggerServer())
	{
		// TODO: Send compilation error
		//m_pRunner->GetDebuggerServer()->
	}

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


void Hail::AngelScript::ErrorHandler::SetActiveScriptRunner(AngelScript::Runner* pRunner)
{
	m_pRunner = pRunner;
}

