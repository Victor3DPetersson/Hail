#pragma once
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"
#include "ShaderCommons.h"

constexpr uint32_t REQUIREDSHADERCOUNT = 4;
class ShaderCompiler;


class ShaderManager
{
public:

	void Update();

	void LoadShader( const char* shaderName);
	bool LoadAllRequiredShaders();
	//Will return null in return value if shaders are not initialized
	GrowingArray<CompiledShader>* GetRequiredShaders();

protected:
	bool CompileRequiredShaders();
	void InitCompiler();
	void DeInitCompiler();
	void Cleanup();

	ShaderCompiler* m_compiler = nullptr;
	GrowingArray<CompiledShader> m_compiledRequiredShaders;
};