#include "Types.h"
#include "Containers\VectorOnStack\VectorOnStack.h"

#include "AngelScriptDebuggerTypes.h"
#include "AngelScriptDebuggerMessagePackager.h"
class asIScriptEngine;
class asIScriptContext;

namespace Hail
{
	constexpr uint32 MAX_ATTACHED_DEBUGGERS = 4u;
#ifdef _WIN32 
	typedef unsigned int H_Socket;
#else
	typedef int H_Socket;
#endif


	namespace AngelScript
	{
		struct Script;

		class ScriptDebugger
		{
		public:
			ScriptDebugger(asIScriptContext* pContext);

			void SetScriptContext(asIScriptContext* pContext);
			// Line callback invoked by context
			void LineCallback(asIScriptContext* pContext);

			void SetLineCallback();
			void StopDebugging();
			void AddBreakpoints(const FileBreakPoints& breakPoints);
			void RemoveBreakpoints(const FileBreakPoints& breakPoints);
			void RemoveBreakpoints();
			void SetExecutionStatus(eScriptExecutionStatus status) { m_executionStatus = status; }
			eScriptExecutionStatus GetStatus() const { return m_executionStatus; }

			GrowingArray<DebuggerMessage>& GetMessages() { return m_messages; }

		private:
			DebuggerMessage CreateHitBreakpointMessage(int line, StringL file);
			bool m_bIsDebugging;
			asIScriptContext* m_pScriptContext;
			// TODO add these breakpoints to a hashmap
			GrowingArray<FileBreakPoints> m_breakPoints;
			eScriptExecutionStatus m_executionStatus;
			GrowingArray<DebuggerMessage> m_messages;
		};

		class DebuggerServer
		{
		public:

			DebuggerServer();
			~DebuggerServer();

			void Update();
			void SetScriptToDebug(Script* pScript) { m_pActiveScript = pScript; }
			Script* GetActiveScript() const { return m_pActiveScript; }

			void StartDebugging();
			void StopDebugging();
			void AddBreakpoints(const FileBreakPoints& breakpointsToAdd);
			void ContinueDebugging();
			

		private:

			void SendDebuggerMessage(DebuggerMessage& messageToSend);
			void ListenToMessages();

			struct Client
			{
				bool m_bConnected;
				H_Socket m_socket;
			};
			VectorOnStack<Client, MAX_ATTACHED_DEBUGGERS> m_clients;
			Script* m_pActiveScript;
			H_Socket m_socketHandle;
			uint32 m_currentClient;
			bool m_bIsDebugging;
		};

	}
}


// Keeping the Official AS debugger reference code commented out for future reading and reference
//#ifndef DEBUGGER_H
//#define DEBUGGER_H
//
//#ifndef ANGELSCRIPT_H 
//// Avoid having to inform include path if header is already include before
//#include <angelscript.h>
//#endif
//
//#include <string>
//#include <vector>
//#include <map>
//
//BEGIN_AS_NAMESPACE
//
//class CDebugger
//{
//public:
//	CDebugger();
//	virtual ~CDebugger();
//
//	// Register callbacks to handle to-string conversions of application types
//	// The expandMembersLevel is a counter for how many recursive levels the members should be expanded.
//	// If the object that is being converted to a string has members of its own the callback should call
//	// the debugger's ToString passing in expandMembersLevel - 1.
//	typedef std::string (*ToStringCallback)(void *obj, int expandMembersLevel, CDebugger *dbg);
//	virtual void RegisterToStringCallback(const asITypeInfo *ti, ToStringCallback callback);
//
//	// User interaction
//	virtual void TakeCommands(asIScriptContext *ctx);
//	virtual void Output(const std::string &str);
//
//	// Line callback invoked by context
//	virtual void LineCallback(asIScriptContext *ctx);
//
//	// Commands
//	virtual void PrintHelp();
//	virtual void AddFileBreakPoint(const std::string &file, int lineNbr);
//	virtual void AddFuncBreakPoint(const std::string &func);
//	virtual void ListBreakPoints();
//	virtual void ListLocalVariables(asIScriptContext *ctx);
//	virtual void ListGlobalVariables(asIScriptContext *ctx);
//	virtual void ListMemberProperties(asIScriptContext *ctx);
//	virtual void ListStatistics(asIScriptContext *ctx);
//	virtual void PrintCallstack(asIScriptContext *ctx);
//	virtual void PrintValue(const std::string &expr, asIScriptContext *ctx);
//
//	// Helpers
//	virtual bool InterpretCommand(const std::string &cmd, asIScriptContext *ctx);
//	virtual bool CheckBreakPoint(asIScriptContext *ctx);
//	virtual std::string ToString(void *value, asUINT typeId, int expandMembersLevel, asIScriptEngine *engine);
//
//	// Optionally set the engine pointer in the debugger so it can be retrieved
//	// by callbacks that need it. This will hold a reference to the engine.
//	virtual void SetEngine(asIScriptEngine *engine);
//	virtual asIScriptEngine *GetEngine();
//	
//protected:
//	enum DebugAction
//	{
//		CONTINUE,  // continue until next break point
//		STEP_INTO, // stop at next instruction
//		STEP_OVER, // stop at next instruction, skipping called functions
//		STEP_OUT   // run until returning from current function
//	};
//	DebugAction        m_action;
//	asUINT             m_lastCommandAtStackLevel;
//	asIScriptFunction *m_lastFunction;
//
//	struct BreakPoint
//	{
//		BreakPoint(std::string f, int n, bool _func) : name(f), lineNbr(n), func(_func), needsAdjusting(true) {}
//		std::string name;
//		int         lineNbr;
//		bool        func;
//		bool        needsAdjusting;
//	};
//	std::vector<BreakPoint> m_breakPoints;
//
//	asIScriptEngine *m_engine;
//
//	// Registered callbacks for converting types to strings
//	std::map<const asITypeInfo*, ToStringCallback> m_toStringCallbacks;
//};
//
//END_AS_NAMESPACE
//
//#endif