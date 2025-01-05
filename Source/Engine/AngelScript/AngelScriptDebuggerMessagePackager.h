#pragma once
#include "Types.h"
#include "AngelScriptDebuggerTypes.h"

//This file and its struct must match the Angelscript extensions structs and data for packing and unpacking. 

namespace Hail
{

	namespace AngelScript
	{
		class DebuggerServer;
		enum class eDebuggerMessageType : int16
		{
			Disconnect = 0,
			StopExecution,
			HitBreakpoint, 
			StartDebugSession,
			CreateBreakpoints,
			Paused,
			Stopped,
			Continued,
			CallStack, // If sent from VS code it is a callstack request, if from Client it is a package with callstack information
			VariableRequest,
			EvaluateRequest,
			StepIn,
			StepOver,
			StepOut,
			End
		};

		struct MessageHeader
		{
			int16 messageLength;
			eDebuggerMessageType type;
		};

		struct MessageData
		{
		public:
			char* GetMessageData();
			void FillMessageBuffer(void* dataToFillWith, uint32 numberOfBytesToFill);
		private:
			uint32 m_dataAmount{ 0u };
			char m_message[1024];
			GrowingArray<char> m_extendableMessage;
		};

		struct DebuggerMessage
		{
			MessageHeader m_header;
			MessageData m_data;
		};

		DebuggerMessage CreateStopDebugSessionMessage();
		DebuggerMessage CreateHitBreakpointMessage(int line, const StringL& file);
		DebuggerMessage CreateCallstackMessage(const GrowingArray<StackFrame>& callstackToSend);
		DebuggerMessage CreateVariablesMessage(const GrowingArray<Variable>* variableScopeToSend);
		DebuggerMessage CreateVariableMessage(const Variable* variableToSend);
		DebuggerMessage CreateStopExecutionMessage();

		void HandleDebuggerMessage(DebuggerServer* pDebugger, uint32 messageLength, void* messageStream);
	}
}