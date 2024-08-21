#include "Engine_PCH.h"
#include "AngelScriptRunner.h"

#include "AngelScriptScriptbuilder.h"
#include "AngelScriptDebugger.h"

using namespace Hail;

#include <sstream>


void Hail::AngelScript::Runner::Initialize(asIScriptEngine* pScriptEngine)
{
	m_pScriptEngine = pScriptEngine;
	m_pDebuggerServer = new DebuggerServer();
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
	else
	{
		if (m_pDebuggerServer)
		{
			//TODO: send error messages to connected clients if there are any
		}
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
		//H_ERROR(StringL::Format("No script loaded called %s", scriptName));
		return;
	}

	Script& script = m_scripts[scriptIndex];
	if (m_pDebuggerServer)
		m_pDebuggerServer->SetScriptToDebug(&script);

	if (script.loadStatus != eScriptLoadStatus::NoError)
	{
		H_ASSERT(script.loadStatus != eScriptLoadStatus::FailedToLoad, "How do we have an unloaded script running? Progrmaming error afoot");
		H_ERROR(StringL::Format("Failed to reload script %s", script.m_name));
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
	//script.m_pScriptContext->SetLineCallback(asMETHOD(CDebugger, LineCallback), m_pDebugger, asCALL_THISCALL);
	// Allow the user to initialize the debugging before moving on
	//m_pDebugger->TakeCommands(script.m_pScriptContext);
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
			H_ERROR(StringL::Format("An exception '%s' occurred. Please correct the code and try again.", script.m_pScriptContext->GetExceptionString()));
		}
	}
}

void Hail::AngelScript::Runner::Update()
{
	// Check for new connections and messages on the server if debugging
	if (m_pDebuggerServer)
		m_pDebuggerServer->Update();

	for (int i = 0; i < m_scripts.Size(); i++)
	{
		Script& script = m_scripts[i];

		//Adding a delay to the reloading, as there can be a frame or two where the filesystem is still saving the script. Leading to a load error.
		if (!script.m_isDirty)
		{
			if (script.m_lastWriteTime != m_scripts[i].m_filePath.GetCurrentLastWriteFileTime())
			{
				script.m_isDirty = true;
				script.m_reloadDelay = 0;
			}
		}

		if (script.m_isDirty)
		{
			if (script.m_reloadDelay > 10)
			{
				ReloadScript(script);
				script.m_lastWriteTime = m_scripts[i].m_filePath.GetCurrentLastWriteFileTime();
				script.m_isDirty = false;
			}
			else
				script.m_reloadDelay++;
		}
	}

}

void Hail::AngelScript::Runner::Cleanup()
{
	for (int i = 0; i < m_scripts.Size(); i++)
	{
		if (m_scripts[i].m_pScriptContext != nullptr)
			m_scripts[i].m_pScriptContext->Release();
		SAFEDELETE(m_scripts[i].m_pDebugger);
	}
	m_scripts.RemoveAll();
	SAFEDELETE(m_pDebuggerServer);

#ifdef DEBUG
	//m_pDebugger->SetEngine(nullptr);
	//SAFEDELETE(m_pDebugger)
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
	// TODO add all files that are parts of the script
	char filePathAsChar[1024];
	FromWCharToConstChar(filePath.Data(), filePathAsChar, 1024);
	r = builder.AddSectionFromFile(filePathAsChar);
	if (r < 0)
	{
		// The builder wasn't able to load the file. Maybe the file
		// has been removed, or the wrong name was given, or some
		// preprocessing commands are incorrectly written.
		scriptToFill.loadStatus = eScriptLoadStatus::FailedToLoad;
		return false;
	}
	
	r = builder.BuildModule();
	if (r < 0)
	{
		// An error occurred. Instruct the script writer to fix the 
		// compilation errors that were listed in the output stream.
		std::stringstream stream;
		auto streamBuf = std::cout.rdbuf();
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
	scriptToFill.m_fileNames.RemoveAll();

	//TODO add all script fileNames
	scriptToFill.m_fileNames.Add(filePath.Object().Name().CharString());

	// Create our context, prepare it, and then execute
	asIScriptContext* pCtx = m_pScriptEngine->CreateContext();
	scriptToFill.m_filePath = filePath;
	scriptToFill.m_lastWriteTime = filePath.GetCurrentLastWriteFileTime();
	scriptToFill.m_pScriptContext = pCtx;
	scriptToFill.m_name = scriptName;
	scriptToFill.loadStatus = eScriptLoadStatus::NoError;
	scriptToFill.m_reloadDelay = 0;
	scriptToFill.m_isDirty = false;
	// Set some setting or way to toggle the debugger off.
	scriptToFill.m_pDebugger = new ScriptDebugger(pCtx);

	return true;
}

void Hail::AngelScript::Runner::ReloadScript(Script& scriptToReload)
{
	if (scriptToReload.m_pScriptContext)
	{
		scriptToReload.m_pScriptContext->Release();
		scriptToReload.m_pScriptContext = nullptr;
		if (m_pDebuggerServer && m_pDebuggerServer->GetActiveScript() == &scriptToReload)
			m_pDebuggerServer->SetScriptToDebug(nullptr);
		SAFEDELETE(scriptToReload.m_pDebugger);
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
		H_ERROR(StringL::Format("Failed to reload script %s", scriptToReload.m_name));
	}

}
