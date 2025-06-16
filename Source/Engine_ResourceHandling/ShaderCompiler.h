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
		static eShaderStage CheckShaderType(const char* shaderExtension);

	private:
		bool CompileSpecificShader(const char* shaderName, eShaderStage shaderType);
		void Init(SHADERLANGUAGETARGET shaderCompilationTarget);

		SHADERLANGUAGETARGET m_languageToCompileTo;
	};
}
