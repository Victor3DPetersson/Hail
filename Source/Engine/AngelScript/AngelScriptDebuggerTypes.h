#pragma once
#include "Types.h"

#include "Containers\GrowingArray\GrowingArray.h"
#include "Utility\FilePath.hpp"

class asIScriptContext;

namespace Hail
{
	namespace AngelScript
	{
		class ScriptDebugger;
		class DebuggerServer;

		enum class eScriptLoadStatus
		{
			NoError,
			FailedToLoad,
			FailedToReload
		};

		enum class eScriptExecutionStatus
		{
			Normal,
			StoppedExecution,
			PausedExecution,
			StepIn,
			StepOver,
			StepOut
		};

		class Script
		{
		public:
			Script()
				: m_lastWriteTime(0), m_pScriptContext(nullptr), loadStatus(eScriptLoadStatus::FailedToLoad),
				m_reloadDelay(0), m_bIsDirty(false), m_pDebugger(nullptr)
			{}
			Script(const FilePath& filePath)
				: m_lastWriteTime(0), m_pScriptContext(nullptr), loadStatus(eScriptLoadStatus::FailedToLoad), 
				m_reloadDelay(0), m_bIsDirty(false), m_pDebugger(nullptr), m_filePath(filePath)
			{}

			FilePath m_filePath;
			uint64 m_lastWriteTime;
			asIScriptContext* m_pScriptContext;
			String64 m_name;
			eScriptLoadStatus loadStatus;

			uint32 m_reloadDelay;
			bool m_bIsDirty;
			
			ScriptDebugger* m_pDebugger;
			GrowingArray<String64> m_fileNames;
		};


		struct BreakPoint
		{
			int16 line;
			bool bHasConditional;
			StringL condition;
		};

		struct FileBreakPoints
		{
			StringL fileName;
			GrowingArray<BreakPoint> breakPoints;
		};

		struct StackFrame
		{
			uint32 m_line;
			StringL m_functionName;
			StringL m_sourceFile;
		};

		enum class eCallStack : uint8
		{
			global,
			self, // this
			local,
			count
		};

		struct Variable
		{
			StringL m_name;
			StringL m_value;
			StringL m_type;
			GrowingArray<Variable> m_members;
		};

		typedef Variable(*ToVariableCallback)(void* obj);

	}

}