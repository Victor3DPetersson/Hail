#include "Engine_PCH.h"
#include "AngelScriptRunner.h"

#include "AngelScriptScriptbuilder.h"
#include "AngelScriptDebugger.h"

using namespace Hail;



void Hail::AngelScript::Runner::Initialize(asIScriptEngine* pScriptEngine)
{
	m_pScriptEngine = pScriptEngine;

#ifdef DEBUG
	m_pDebugger = new CDebugger();
	m_pDebugger->SetEngine(m_pScriptEngine);
#endif

}

void Hail::AngelScript::Runner::ImportAndBuildScript(const FilePath& filePath, String64 scriptName)
{
	for (int i = 0; i < m_scripts.Size(); i++)
	{
		if (m_scripts[i].m_filePath == filePath)
			return;
	}

	Script newScript;

	if (CreateScript(filePath, scriptName, newScript))
	{
		m_scripts.Add(newScript);
	}

}

void Hail::AngelScript::Runner::RunScript(String64 scriptName)
{
	H_ASSERT(m_pScriptEngine, "Must have a script engine on the script runner.");

	int scriptIndex = -1;

	for (int i = 0; i < m_scripts.Size(); i++)
	{
		if (m_scripts[i].m_name == scriptName)
			scriptIndex = i;
	}

	if (scriptIndex < 0)
	{
		H_ERROR(String256::Format("No script loaded called %s", scriptName));
		return;
	}

	Script& script = m_scripts[scriptIndex];

	if (script.loadStatus != eScriptLoadStatus::NoError)
	{
		H_ASSERT(script.loadStatus != eScriptLoadStatus::FailedToLoad, "How do we have an unloaded script running? Progrmaming error afoot");
		H_ERROR(String256::Format("Failed to reload script %s", script.m_name));
		return;
	}

	asIScriptModule* mod = m_pScriptEngine->GetModule("MyModule");
	asIScriptFunction* func = mod->GetFunctionByDecl("void main()");
	if (func == 0)
	{
		// The function couldn't be found. Instruct the script writer
		// to include the expected function in the script.
		H_ERROR("The script must have the function 'void main()'. Please add it and try again.");
		return;
	}

	script.m_pScriptContext->Prepare(func);

#ifdef DEBUG
	// Tell the context to invoke the debugger's line callback
	script.m_pScriptContext->SetLineCallback(asMETHOD(CDebugger, LineCallback), m_pDebugger, asCALL_THISCALL);
	// Allow the user to initialize the debugging before moving on
	m_pDebugger->TakeCommands(script.m_pScriptContext);
	// Execute the script normally. If a breakpoint is reached the 
	// debugger will take over the control loop.
	int r = script.m_pScriptContext->Execute();
#else
	int r = script.m_pScriptContext->Execute();
#endif

	if (r != asEXECUTION_FINISHED)
	{
		// The execution didn't complete as expected. Determine what happened.
		if (r == asEXECUTION_EXCEPTION)
		{
			// An exception occurred, let the script writer know what happened so it can be corrected.
			H_ERROR(String256::Format("An exception '%s' occurred. Please correct the code and try again.", script.m_pScriptContext->GetExceptionString()));
		}
	}
}

void Hail::AngelScript::Runner::Update()
{

	for (int i = 0; i < m_scripts.Size(); i++)
	{
		if (m_scripts[i].m_lastWriteTime != m_scripts[i].m_filePath.GetCurrentLastWriteFileTime())
		{
			ReloadScript(m_scripts[i]);
			m_scripts[i].m_lastWriteTime = m_scripts[i].m_filePath.GetCurrentLastWriteFileTime();
		}
	}

}

void Hail::AngelScript::Runner::Cleanup()
{
	for (int i = 0; i < m_scripts.Size(); i++)
	{
		if (m_scripts[i].m_pScriptContext != nullptr)
			m_scripts[i].m_pScriptContext->Release();
	}
	m_scripts.RemoveAll();

#ifdef DEBUG
	m_pDebugger->SetEngine(nullptr);
	SAFEDELETE(m_pDebugger)
#endif
}

bool Hail::AngelScript::Runner::CreateScript(const FilePath& filePath, String64 scriptName, Script& scriptToFill)
{
	if (!filePath.IsValid())
	{
		H_ERROR("Invalid script path");
		scriptToFill.loadStatus = eScriptLoadStatus::FailedToLoad;
		return false;
	}

	H_ASSERT(m_pScriptEngine, "Must have a script engine on the importer.");
	CScriptBuilder builder;

	int r = builder.StartNewModule(m_pScriptEngine, "MyModule");
	if (r < 0)
	{
		// If the code fails here it is usually because there
		// is no more memory to allocate the module
		H_ERROR("Unrecoverable error while starting a new module.");
		scriptToFill.loadStatus = eScriptLoadStatus::FailedToLoad;
		return false;
	}
	char filePathAsChar[1024];
	FromWCharToConstChar(filePath.Data(), filePathAsChar, 1024);
	r = builder.AddSectionFromFile(filePathAsChar);
	if (r < 0)
	{
		// The builder wasn't able to load the file. Maybe the file
		// has been removed, or the wrong name was given, or some
		// preprocessing commands are incorrectly written.
		H_ERROR("Please correct the errors in the script and try again.");
		scriptToFill.loadStatus = eScriptLoadStatus::FailedToLoad;
		return false;
	}
	r = builder.BuildModule();
	if (r < 0)
	{
		// An error occurred. Instruct the script writer to fix the 
		// compilation errors that were listed in the output stream.
		H_ERROR("Please correct the errors in the script and try again.");
		scriptToFill.loadStatus = eScriptLoadStatus::FailedToLoad;
		return false;
	}

	asIScriptModule* mod = m_pScriptEngine->GetModule("MyModule");
	asIScriptFunction* func = mod->GetFunctionByDecl("void main()");
	if (func == 0)
	{
		// The function couldn't be found. Instruct the script writer
		// to include the expected function in the script.
		H_ERROR("The script must have the function 'void main()'. Please add it and try again.");
		scriptToFill.loadStatus = eScriptLoadStatus::FailedToLoad;
		return false;
	}

	// Create our context, prepare it, and then execute
	asIScriptContext* pCtx = m_pScriptEngine->CreateContext();

	scriptToFill.m_filePath = filePath;
	scriptToFill.m_lastWriteTime = filePath.GetCurrentLastWriteFileTime();
	scriptToFill.m_pScriptContext = pCtx;
	scriptToFill.m_name = scriptName;
	scriptToFill.loadStatus = eScriptLoadStatus::NoError;
	return true;
}

void Hail::AngelScript::Runner::ReloadScript(Script& scriptToReload)
{
	if (scriptToReload.m_pScriptContext)
	{
		scriptToReload.m_pScriptContext->Release();
		scriptToReload.m_pScriptContext = nullptr;
	}
	Script newScript;
	if (CreateScript(scriptToReload.m_filePath, scriptToReload.m_name, newScript))
	{
		scriptToReload = newScript;
		H_DEBUGMESSAGE("Reload A_OK! c:");
	}
	else
	{
		scriptToReload.loadStatus = eScriptLoadStatus::FailedToReload;
		H_ERROR(String256::Format("Failed to reload script %s", scriptToReload.m_name));
	}

}
