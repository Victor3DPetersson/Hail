//Interface for the entire engine
#pragma once

#include "ShaderCommons.h"
#include "Containers\GrowingArray\GrowingArray.h"


namespace Hail
{
	class ShaderManager;
	class ShaderCompiler
	{
	public:
		friend class MaterialManager;

		void CompileAllShaders();
		bool CompileAndExportAllRequiredShaders(const char** requiredShaders, uint32_t numberOfRequiredShaders);
		bool CompileSpecificShader(const char* shaderName, SHADERTYPE shaderType);
		bool IsShaderCompiled(const char* relativePath, const char* shaderName);

	private:

		void Init(SHADERLANGUAGETARGET shaderCompilationTarget);


		SHADERTYPE CheckShaderType(const char* shaderExtension);

		SHADERLANGUAGETARGET m_languageToCompileTo;
	};
}
