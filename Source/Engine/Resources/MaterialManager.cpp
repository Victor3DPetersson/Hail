#include "Engine_PCH.h"
#include "MaterialManager.h"
#include "ShaderCompiler.h"

#include "DebugMacros.h"
#include "Utility\InOutStream.h"

namespace Hail
{

	void MaterialManager::Update()
	{
	}
	void MaterialManager::Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain)
	{
		m_renderDevice = renderingDevice;
		m_textureManager = textureResourceManager;
		m_swapChain = swapChain;
		m_renderingResourceManager = renderingResourceManager;
		m_materialsInstanceData.Init(10);
	}

	bool MaterialManager::InitMaterial(MATERIAL_TYPE type, FrameBufferTexture* frameBufferToBindToMaterial)
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
		m_materials[(uint32_t)(type)] = material;

		return InitMaterialInternal(type, frameBufferToBindToMaterial);
	}

	Material& MaterialManager::GetMaterial(MATERIAL_TYPE materialType)
	{
		return m_materials[static_cast<uint32_t>(materialType)];
	}

	const MaterialInstance& MaterialManager::GetMaterialInstance(uint32_t instanceID)
	{
		return m_materialsInstanceData[instanceID];
	}

	bool MaterialManager::CreateInstance(MATERIAL_TYPE materialType, MaterialInstance instanceData)
	{
		instanceData.m_instanceIdentifier = m_materialsInstanceData.Size();
		instanceData.m_materialIdentifier = static_cast<uint32_t>(materialType);
		m_materialsInstanceData.Add(instanceData);
		return InitMaterialInstanceInternal(m_materials[(uint32_t)(materialType)], m_materialsInstanceData.GetLast());
	}

	//TODO: Add relative path support in the shader output
	CompiledShader MaterialManager::LoadShader(const char* shaderName, SHADERTYPE shaderType)
	{
		CompiledShader shader{};
		String256 inPath = String256::Format("%s%s%s", SHADER_DIR_OUT, shaderName, ".shr");

		InOutStream inStream;

		if (!inStream.OpenFile(inPath.Data(), FILE_OPEN_TYPE::READ, true))
		{
			InitCompiler();
			if (!m_compiler->CompileSpecificShader(shaderName, shaderType))
			{
				return shader;
			}
			DeInitCompiler();
		}
		inStream.OpenFile(inPath.Data(), FILE_OPEN_TYPE::READ, true);

		Debug_PrintConsoleString256(String256::Format("\nImporting Shader:\n%s:", shaderName));

		inStream.Read((char*)&shader.header, sizeof(ShaderHeader));
		Debug_PrintConsoleString256(String256::Format("Shader Size:%i:%s", shader.header.sizeOfShaderData, "\n"));

		shader.compiledCode = new char[shader.header.sizeOfShaderData];
		inStream.Read((char*)shader.compiledCode, sizeof(char) * shader.header.sizeOfShaderData);
		inStream.Read(shader.compiledCode, shader.header.sizeOfShaderData);

		inStream.CloseFile();

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

	void MaterialManager::ClearAllResources()
	{
	}
}
