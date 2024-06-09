//Interface for the entire engine
#pragma once

#include "Resources_Materials\ShaderCommons.h"
#include "Containers\GrowingArray\GrowingArray.h"


namespace Hail
{
	class ShaderManager;
	class ShaderCompiler
	{
	public:
		friend class MaterialManager;

	private:
		bool CompileSpecificShader(const char* shaderName, eShaderType shaderType);
		eShaderType CheckShaderType(const char* shaderExtension);
		void Init(SHADERLANGUAGETARGET shaderCompilationTarget);

		SHADERLANGUAGETARGET m_languageToCompileTo;
	};
}
