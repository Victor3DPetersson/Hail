#include "Engine_PCH.h"
#include "MaterialManager.h"
#include "ShaderCompiler.h"

#include <filesystem>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "DebugMacros.h"

namespace Hail
{

	void MaterialManager::Update()
	{
	}
	//Index is temp
	bool MaterialManager::LoadMaterial(MATERIAL_TYPE type)
	{
		Material material;
		//Get this Data from a data file
		switch (type)
		{
		case Hail::MATERIAL_TYPE::SPRITE:
			material.m_vertexShader = LoadShader("VS_Sprite", SHADERTYPE::VERTEX);
			material.m_fragmentShader = LoadShader("FS_Sprite", SHADERTYPE::FRAGMENT);
			break;
		case Hail::MATERIAL_TYPE::FULLSCREEN_PRESENT_LETTERBOX:
			material.m_vertexShader = LoadShader("VS_fullscreenPass", SHADERTYPE::VERTEX);
			material.m_fragmentShader = LoadShader("FS_fullscreenPass", SHADERTYPE::FRAGMENT);
			break;
		case Hail::MATERIAL_TYPE::MODEL3D:
			material.m_vertexShader = LoadShader("VS_triangle", SHADERTYPE::VERTEX);
			material.m_fragmentShader = LoadShader("FS_triangle", SHADERTYPE::FRAGMENT);
			break;
		case Hail::MATERIAL_TYPE::COUNT:
			break;
		default:
			break;
		}
		if (material.m_vertexShader.loadState != SHADER_LOADSTATE::LOADED_TO_RAM ||
			material.m_fragmentShader.loadState != SHADER_LOADSTATE::LOADED_TO_RAM)
		{
			return false;
		}

		///VERY TEMP ABOVE ^
		material.m_type = type;
		m_materials[static_cast<uint32_t>(type)] = material;
		m_materialsInstanceData[static_cast<uint32_t>(type)].Init(10);
		return true;
	}

	Material& MaterialManager::GetMaterial(MATERIAL_TYPE materialType)
	{
		return m_materials[static_cast<uint32_t>(materialType)];
	}

	const MaterialInstance& MaterialManager::GetMaterialInstance(MATERIAL_TYPE materialType, uint32_t instanceID)
	{
		return m_materialsInstanceData[static_cast<uint32_t>(materialType)][instanceID];
	}

	MaterialInstance& MaterialManager::CreateInstance(MATERIAL_TYPE materialType)
	{
		MaterialInstance instance;
		instance.m_instanceIdentifier = m_materialsInstanceData[static_cast<uint32_t>(materialType)].Size();
		instance.m_materialIdentifier = static_cast<uint32_t>(materialType);
		m_materialsInstanceData[static_cast<uint32_t>(materialType)].Add(instance);
		return m_materialsInstanceData[static_cast<uint32_t>(materialType)].GetLast();
	}

	//TODO: Add relative path support in the shader output
	CompiledShader MaterialManager::LoadShader(const char* shaderName, SHADERTYPE shaderType)
	{
		CompiledShader shader{};
		String256 inPath = String256::Format("%s%s%s", SHADER_DIR_OUT, shaderName, ".shr");

		std::ifstream inStream(inPath.Data(), std::ios::in | std::ios::binary);
		if (!inStream)
		{
			InitCompiler();
			if (!m_compiler->CompileSpecificShader(shaderName, shaderType))
			{
				return shader;
			}
			DeInitCompiler();
			inStream.open(inPath.Data(), std::ios::in | std::ios::binary);
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
		return shader;
	}

	void MaterialManager::InitCompiler()
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

	void MaterialManager::DeInitCompiler()
	{
		SAFEDELETE(m_compiler)
	}

	void MaterialManager::Cleanup()
	{
	}
}