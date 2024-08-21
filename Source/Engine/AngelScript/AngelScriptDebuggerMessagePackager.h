#pragma once
#include "Types.h"

//This file and its struct must match the Angelscript extensions structs and data for packing and unpacking. 

namespace Hail
{

	namespace AngelScript
	{
		class DebuggerServer;
		enum class eDebuggerMessageType : int16
		{
			Disconnect = 0,
			HitBreakpoint, 
			// Below are sent from VS-Code to Hail, above sent from Hail
			StartDebugSession,
			CreateBreakpoints,
			Stopped,
			Continued,
			Step,
			End
		};

		struct MessageHeader
		{
			int16 messageLength;
			eDebuggerMessageType type;
		};

		struct DebuggerMessage
		{
			MessageHeader m_header;
			char m_message[1024];
		};

		void HandleDebuggerMessage(DebuggerServer* pDebugger, uint32 messageLength, void* messageStream);
	}
}