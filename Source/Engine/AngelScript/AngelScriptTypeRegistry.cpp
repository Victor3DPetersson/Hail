#include "Engine_PCH.h"
#include "AngelScriptTypeRegistry.h"
#include "angelscript.h"

Hail::AngelScript::TypeRegistry::TypeRegistry(asIScriptEngine* pAsEngine)
	: m_pScriptEngine(pAsEngine)
{
}

bool Hail::AngelScript::TypeRegistry::RegisterType(const char* typeName, uint32 sizeOfType, uint64 flags)
{
	for (uint32 i = 0; i < m_registeredTypes.Size(); i++)
	{
		if (m_registeredTypes[i].nameID == typeName)
		{
			H_ERROR(StringL::Format("Already awssigned angelscript object %s: ", typeName));
			return true;
		}
	}

	AsTypeIDNamePair objectToRegister;
	objectToRegister.nameID = typeName;
	objectToRegister.bRegisteredVariableFetchFunction = false;

	int r = m_pScriptEngine->RegisterObjectType(typeName, sizeOfType, flags); assert(r >= 0);
	objectToRegister.typeID = m_pScriptEngine->GetTypeIdByDecl(typeName);
	m_registeredTypes.Add(objectToRegister);

    return r >= 0;
}

bool Hail::AngelScript::TypeRegistry::RegisterVariableFunction(const char* typeName, ToVariableCallback callbackToRegister)
{
	for (uint32 i = 0; i < m_registeredTypes.Size(); i++)
	{
		if (StringCompare(m_registeredTypes[i].nameID, typeName))
		{
			m_registeredTypes[i].toVariableCallback = callbackToRegister;
			m_registeredTypes[i].bRegisteredVariableFetchFunction = true;
			return true;
		}
	}

	H_ERROR("Trying to assign a ToVariable callback to an unregistered object type.");

    return false;
}

Hail::AngelScript::Variable Hail::AngelScript::TypeRegistry::GetVariableFromCallback(uint32 typeID, void* pObject)
{
	for (uint32 i = 0; i < m_registeredTypes.Size(); i++)
	{
		if (m_registeredTypes[i].typeID == typeID)
		{
			H_ASSERT(m_registeredTypes[i].bRegisteredVariableFetchFunction, StringL::Format("Unregistered callback invoked, add a callbasck to type %s", m_registeredTypes[i].nameID.Data()));
			return m_registeredTypes[i].toVariableCallback(pObject);
		}
	}
	const char* declaration = m_pScriptEngine->GetTypeDeclaration(typeID);
	const uint32 declarationLength = StringLength(declaration);
	if (declaration[declarationLength - 2] == '[' && declaration[declarationLength - 1] == ']')
	{
		// 0 is for arrays
		return m_registeredTypes[0].toVariableCallback(pObject);
	}

	return Variable();
}
