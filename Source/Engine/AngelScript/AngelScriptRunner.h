#pragma once
#include "Utility\FilePath.hpp"
#include "Containers\GrowingArray\GrowingArray.h"

class asIScriptEngine;
class asIScriptContext;
class CDebugger;

namespace Hail
{
	class FilePath;
	namespace AngelScript
	{
		enum class eScriptLoadStatus
		{
			NoError,
			FailedToLoad,
			FailedToReload
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
		};

		class Runner
		{
		public:

			void ImportAndBuildScript(const FilePath& filePath, String64 scriptName);

			void Initialize(asIScriptEngine* pScriptEngine);

			void RunScript(String64 scriptName);
			// Will iterate over the loaded scripts and reload the ones that are out of date.
			void Update();

			void Cleanup();

		private:
			// Will return true if the creation is succesfull 
			bool CreateScript(const FilePath& filePath, String64 scriptName, Script& scriptToFill);

			void ReloadScript(Script& scriptToReload);

			asIScriptEngine* m_pScriptEngine;
			GrowingArray<Script> m_scripts;
			CDebugger* m_pDebugger;
		};

	}


}