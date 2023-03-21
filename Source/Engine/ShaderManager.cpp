#include "Engine_PCH.h"
#include "ShaderManager.h"
#include "ShaderCompiler.h"

#include <filesystem>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "DebugMacros.h"


const char* requiredShaders[REQUIREDSHADERCOUNT] =
{
	"VS_triangle",
	"FS_triangle",
	"VS_fullscreenPass",
	"FS_fullscreenPass",
	"VS_Sprite",
	"FS_Sprite"
};


void ShaderManager::Update()
{
}

//TODO: Add relative path support in the shader output
void ShaderManager::LoadShader(const char* shaderName)
{
	CompiledShader shader{};
	String256 inPath = String256::Format("%s%s%s", SHADER_DIR_OUT, shaderName, ".shr");

	std::ifstream inStream(inPath.Data(), std::ios::in | std::ios::binary);
	if (!inStream)
	{
		return;
	}

	Debug_PrintConsoleString256(String256::Format("\nImporting Shader:\n%s:", shaderName));

	inStream.read((char*)&shader.header, sizeof(ShaderHeader));
	Debug_PrintConsoleString256(String256::Format("Shader Size:%i:%s", shader.header.sizeOfShaderData, "\n"));

	shader.compiledCode = new char[shader.header.sizeOfShaderData];
	inStream.read((char*)shader.compiledCode, sizeof(char) * shader.header.sizeOfShaderData);
	inStream.read(shader.compiledCode, shader.header.sizeOfShaderData);


	inStream.close();

	shader.loadState = SHADER_LOADSTATE::LOADED_TO_RAM;
	shader.shaderName = shaderName;
	m_compiledRequiredShaders.Add(shader);
}

bool ShaderManager::LoadAllRequiredShaders()
{
	GrowingArray<String256> foundCompiledShaders(REQUIREDSHADERCOUNT);
	std::filesystem::path pathToShow{ SHADER_DIR_OUT };
	if (std::filesystem::exists(pathToShow))
	{
		Debug_PrintConsoleString64(String64("shaders:    "));
		for (const auto& entry : std::filesystem::directory_iterator(pathToShow))
		{
			const auto filenameStr = entry.path().filename().replace_extension().string();
			if (entry.is_directory())
			{
				Debug_PrintConsoleString256(String256::Format("%s%s", "\tdir:  ", filenameStr.c_str()));
			}
			else if (entry.is_regular_file())
			{
				Debug_PrintConsoleString256(String256::Format("%s%s", "\tfile: ", filenameStr.c_str()));
				foundCompiledShaders.Add(filenameStr);
			}
		}
	}
	uint32_t foundCounter = 0;
	for (uint32_t shader = 0; shader < foundCompiledShaders.Size(); shader++)
	{
		bool foundShader = false;
		for (uint32_t i = 0; i < REQUIREDSHADERCOUNT; i++)
		{
			if (strcmp(requiredShaders[i], foundCompiledShaders[shader].Data()) == 0)
			{
				foundShader = true;
				foundCounter++;
			}
		}
		if (!foundShader)
		{
			foundCompiledShaders.RemoveCyclicAtIndex(shader);
			shader--;
		}
	}
	m_compiledRequiredShaders.Init(REQUIREDSHADERCOUNT);
	if (foundCounter == REQUIREDSHADERCOUNT)
	{
		for (size_t i = 0; i < REQUIREDSHADERCOUNT; i++)
		{
			LoadShader(foundCompiledShaders[i]);
		}
		return true;
	}
	return CompileRequiredShaders();
}

GrowingArray<CompiledShader>* ShaderManager::GetRequiredShaders()
{
	if (m_compiledRequiredShaders.IsInitialized() && m_compiledRequiredShaders.Size() == REQUIREDSHADERCOUNT)
	{
		return  &m_compiledRequiredShaders;
	}
	return nullptr;
}

bool ShaderManager::CompileRequiredShaders()
{
	InitCompiler();
	bool compilationSuccess = m_compiler->CompileAndExportAllRequiredShaders(requiredShaders, REQUIREDSHADERCOUNT);
	if (compilationSuccess)
	{
		for (size_t i = 0; i < REQUIREDSHADERCOUNT; i++)
		{
			LoadShader(requiredShaders[i]);
		}
	}
	DeInitCompiler();
	return compilationSuccess;
}

void ShaderManager::InitCompiler()
{
	if (m_compiler)
	{
		return;
	}
	m_compiler = new ShaderCompiler();
#ifdef PLATFORM_WINDOWS
	m_compiler->Init(SHADERLANGUAGETARGET::GLSL);
#elif PLATFORM_OSX
	compiler.Init(SHADERLANGUAGETARGET::METAL);
#endif 
}

void ShaderManager::DeInitCompiler()
{
	SAFEDELETE(m_compiler)
}

void ShaderManager::Cleanup()
{
	m_compiledRequiredShaders.DeleteAll();
}