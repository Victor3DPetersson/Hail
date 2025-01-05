#pragma once
#include "Utility\FilePath.hpp"
#include "Containers\GrowingArray\GrowingArray.h"
#include "AngelScriptDebuggerTypes.h"

class asIScriptEngine;
class asIScriptContext;

namespace Hail
{
	class FilePath;
	namespace AngelScript
	{
		class ScriptDebugger;
		class DebuggerServer;
		class TypeRegistry;

		class Runner
		{
		public:

			void ImportAndBuildScript(const FilePath& filePath, String64 scriptName);

			void Initialize(asIScriptEngine* pScriptEngine, TypeRegistry* pTypeRegistry);

			void RunScript(String64 scriptName);
			// Will iterate over the loaded scripts and reload the ones that are out of date.
			void Update();

			void Cleanup();

		private:
			// Will return true if the creation is succesfull 
			bool CreateScript(String64 scriptName, Script& scriptToFill);

			void ReloadScript(Script& scriptToReload);

			asIScriptEngine* m_pScriptEngine;
			DebuggerServer* m_pDebuggerServer;
			TypeRegistry* m_pTypeRegistry;
			GrowingArray<Script> m_scripts;
		};

	}


}