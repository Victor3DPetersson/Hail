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

		enum class eScriptLoadStatus
		{
			NoError,
			FailedToLoad,
			FailedToReload
		};

		enum class eScriptExecutionStatus
		{
			Normal,
			HitBreakpoint
		};

		struct Script
		{
			FilePath m_filePath;
			uint64 m_lastWriteTime;
			asIScriptContext* m_pScriptContext;
			String64 m_name;
			eScriptLoadStatus loadStatus;

			uint32 m_reloadDelay;
			bool m_isDirty;
			
			ScriptDebugger* m_pDebugger;

			GrowingArray<String64> m_fileNames;
			// growing array included filepaths, add to filewatcher, callback fetches and fills dis
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
	}

}