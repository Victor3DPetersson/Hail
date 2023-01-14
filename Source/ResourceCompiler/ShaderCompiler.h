//Interface for the entire engine
#pragma once

#include "ShaderCommons.h"
#include "Containers\GrowingArray\GrowingArray.h"


class ShaderManager;
class ShaderCompiler
{
public:
	friend class ShaderManager;

	void CompileAllShaders();
	bool CompileAndExportAllRequiredShaders(const char** requiredShaders, uint32_t numberOfRequiredShaders);
	void CompileSpecificShader(const char* relativePath, const char* shaderName);
	bool IsShaderCompiled(const char* relativePath, const char* shaderName);

private:

	void Init(SHADERLANGUAGETARGET shaderCompilationTarget);


	SHADERTYPE CheckShaderType(const char* shaderExtension);

	SHADERLANGUAGETARGET m_languageToCompileTo;
};