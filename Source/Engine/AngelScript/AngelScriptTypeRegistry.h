#pragma once
#include "Types.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"
#include "AngelScriptDebuggerTypes.h"

class asIScriptEngine;

namespace Hail
{
	namespace AngelScript
	{
		constexpr uint32 NumberOfRegisteredTypes = 2u;
		class TypeRegistry
		{
		public:
			TypeRegistry(asIScriptEngine* pAsEngine);

			bool RegisterType(const char* typeName, uint32 sizeOfType, uint64 flags);
			bool RegisterVariableFunction(const char* typeName, ToVariableCallback callbackToRegister);

			Variable GetVariableFromCallback(uint32 typeID, void* pObject);
			asIScriptEngine* GetEngine() { return m_pScriptEngine; }
		private:

			struct AsTypeIDNamePair
			{
				uint32 typeID;
				StringL nameID;
				bool bRegisteredVariableFetchFunction;
				ToVariableCallback toVariableCallback;
			};

			GrowingArray<AsTypeIDNamePair> m_registeredTypes;

			GrowingArray<ToVariableCallback> m_registeredToCallBackFunctions;
			//TODO: When we have a hashmap, register hashmap with TypeID to variable function ptrs

			asIScriptEngine* m_pScriptEngine;
		};
	}
}