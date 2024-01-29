#include "Engine_PCH.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "ShaderCompiler.h"

#include "DebugMacros.h"
#include "Utility\InOutStream.h"
#include "HailEngine.h"
#include "ResourceRegistry.h"

#include "ImGui\ImGuiContext.h"

namespace Hail
{

	void MaterialManager::Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain)
	{
		m_renderDevice = renderingDevice;
		m_textureManager = textureResourceManager;
		m_swapChain = swapChain;
		m_renderingResourceManager = renderingResourceManager;
		m_materialsInstanceData.Init(10);
		m_materialsInstanceValidationData.Init(10);
	}

	bool MaterialManager::InitMaterial(MATERIAL_TYPE type, FrameBufferTexture* frameBufferToBindToMaterial, bool reloadShader, uint32 frameInFlight)
	{
		if (m_materials[(uint32_t)type].m_fragmentShader.loadState == SHADER_LOADSTATE::UNLOADED
			&& m_materials[(uint32_t)type].m_vertexShader.loadState == SHADER_LOADSTATE::UNLOADED)
		{
			Material material;
			//Get this Data from a data file
			switch (type)
			{
			case MATERIAL_TYPE::SPRITE:
				material.m_vertexShader = LoadShader("VS_Sprite", SHADERTYPE::VERTEX, reloadShader);
				material.m_fragmentShader = LoadShader("FS_Sprite", SHADERTYPE::FRAGMENT, reloadShader);
				break;
			case MATERIAL_TYPE::FULLSCREEN_PRESENT_LETTERBOX:
				material.m_vertexShader = LoadShader("VS_fullscreenPass", SHADERTYPE::VERTEX, reloadShader);
				material.m_fragmentShader = LoadShader("FS_fullscreenPass", SHADERTYPE::FRAGMENT, reloadShader);
				break;
			case MATERIAL_TYPE::MODEL3D:
				material.m_vertexShader = LoadShader("VS_triangle", SHADERTYPE::VERTEX, reloadShader);
				material.m_fragmentShader = LoadShader("FS_triangle", SHADERTYPE::FRAGMENT, reloadShader);
				break;
			case MATERIAL_TYPE::DEBUG_LINES2D:
			case MATERIAL_TYPE::DEBUG_LINES3D:
				material.m_vertexShader = LoadShader("VS_DebugLines", SHADERTYPE::VERTEX, reloadShader);
				material.m_fragmentShader = LoadShader("FS_DebugLines", SHADERTYPE::FRAGMENT, reloadShader);
				break;
			case MATERIAL_TYPE::COUNT:
				break;
			default:
				break;
			}
			if (material.m_vertexShader.loadState != SHADER_LOADSTATE::LOADED_TO_RAM ||
				material.m_fragmentShader.loadState != SHADER_LOADSTATE::LOADED_TO_RAM)
			{
				return false;
			}

			material.m_type = type;
			m_materials[(uint32_t)type] = material;
		}
		///VERY TEMP ABOVE ^

		if (InitMaterialInternal(type, frameBufferToBindToMaterial, frameInFlight))
		{
			m_materialValidators[(uint32)type].ClearFrameData(frameInFlight);
			return true;
		}
		return false;
	}

	Material& MaterialManager::GetMaterial(MATERIAL_TYPE materialType)
	{
		return m_materials[(uint32)materialType];
	}

	const MaterialInstance& MaterialManager::GetMaterialInstance(uint32_t instanceID, MATERIAL_TYPE materialType)
	{
		if (instanceID >= m_materialsInstanceData.Size())
		{
			if (materialType == MATERIAL_TYPE::SPRITE)
				return m_defaultSpriteMaterialInstance;
			if (materialType == MATERIAL_TYPE::MODEL3D)
				return m_default3DMaterialInstance;
		}

		return m_materialsInstanceData[instanceID];
	}

	uint32 MaterialManager::CreateInstance(MATERIAL_TYPE materialType, MaterialInstance instanceData)
	{
		instanceData.m_instanceIdentifier = m_materialsInstanceData.Size();
		instanceData.m_materialIdentifier = (uint32_t)materialType;
		m_materialsInstanceData.Add(instanceData);
		ResourceValidator instanceValidator = ResourceValidator();
		instanceValidator.MarkResourceAsDirty(0);
		m_materialsInstanceValidationData.Add(instanceValidator);
		return instanceData.m_instanceIdentifier;
	}

	Hail::uint32 MaterialManager::GetMaterialInstanceHandle(GUID guid) const
	{
		if (guid == guidZero)
			return MAX_UINT;

		for (uint32 i = 0; i < m_materialsInstanceData.Size(); i++)
		{
			if (guid == m_materialsInstanceData[i].m_id)
				return m_materialsInstanceData[i].m_instanceIdentifier;
		}

		return MAX_UINT;
	}

	bool MaterialManager::InitMaterialInstance(uint32 instanceID, uint32 frameInFlight)
	{
		if (InitMaterialInstanceInternal(m_materialsInstanceData[instanceID], frameInFlight, false))
		{
			m_materialsInstanceValidationData[instanceID].ClearFrameData(frameInFlight); 
			return true;
		}
		return false;
	}

	bool MaterialManager::ReloadAllMaterials(uint32 frameInFlight)
	{
		bool result = true;
		for (uint32_t i = 0; i < (uint32)(MATERIAL_TYPE::COUNT); i++)
		{
			const bool firstFrameOfReload = m_materialValidators[i].GetIsResourceDirty() == false;
			if(firstFrameOfReload)
			{
				ClearHighLevelMaterial((MATERIAL_TYPE)i, frameInFlight);
			}
			ClearMaterialInternal((MATERIAL_TYPE)i, frameInFlight);
			if (InitMaterial((MATERIAL_TYPE)i, nullptr, true, frameInFlight))
			{
				m_materialValidators[i].ClearFrameData(frameInFlight);
			}
		}

		for (uint32 i = 0; i < m_materialsInstanceData.Size(); i++)
		{
			m_materialsInstanceValidationData[i].MarkResourceAsDirty(frameInFlight);
			if (InitMaterialInstanceInternal(m_materialsInstanceData[i], frameInFlight, false))
			{
				m_materialsInstanceValidationData[i].ClearFrameData(frameInFlight);
				result = false;
			}
		}

		return result;
	}

	bool MaterialManager::ReloadAllMaterialInstances(uint32 frameInFlight)
	{
		CheckMaterialInstancesToReload(frameInFlight);
		bool result = true;
		for (uint32 i = 0; i < m_materialsInstanceData.Size(); i++)
		{
			if (m_materialsInstanceValidationData[i].GetIsFrameDataDirty(frameInFlight))
			{
				if (InitMaterialInstanceInternal(m_materialsInstanceData[i], frameInFlight, false))
				{
					m_materialsInstanceValidationData[i].ClearFrameData(frameInFlight);
					result = false;
				}
			}
		}
		return false;
	}

	void MaterialManager::ClearHighLevelMaterial(MATERIAL_TYPE materialType, uint32 frameInFlight)
	{
		//TODO make more proper and robust validation later =) 
		m_materials[(uint32)materialType].m_fragmentShader.loadState = SHADER_LOADSTATE::UNLOADED;
		m_materials[(uint32)materialType].m_vertexShader.loadState = SHADER_LOADSTATE::UNLOADED;
		m_materialValidators[(uint32)materialType].MarkResourceAsDirty(frameInFlight);

	}

	void MaterialManager::CheckMaterialInstancesToReload(uint32 frameInFlight)
	{
		//TODO: potentionally quite slow, but is this a problem as it is only done for reload? 
		const GrowingArray<ResourceValidator>& textureValidators = m_textureManager->GetTextureValidators();
		for (size_t textureID = 0; textureID < textureValidators.Size(); textureID++)
		{
			//If the texture is marked dirty this frame
			if (textureValidators[textureID].GetIsResourceDirty())
			{
				for (uint32 instance = 0; instance < m_materialsInstanceData.Size(); instance++)
				{
					//if we are not already reloading this texture
					if (m_materialsInstanceValidationData[instance].GetIsResourceDirty())
					{
						continue;
					}
					for (uint32 instanceTexture = 0; instanceTexture < MAX_TEXTURE_HANDLES; instanceTexture++)
					{
						if(m_materialsInstanceData[instance].m_textureHandles[instanceTexture] == textureID)
						{
							m_materialsInstanceValidationData[instance].MarkResourceAsDirty(frameInFlight);
							break;
						}
					}
				}
			}
		}
	}

	void MaterialManager::InitDefaultMaterialInstances()
	{
		m_defaultSpriteMaterialInstance.m_instanceIdentifier = MAX_UINT;
		m_defaultSpriteMaterialInstance.m_materialIdentifier = (uint32)MATERIAL_TYPE::SPRITE;
		m_defaultSpriteMaterialsInstanceValidationData.MarkResourceAsDirty(0);

		m_default3DMaterialInstance.m_instanceIdentifier = MAX_UINT;
		m_default3DMaterialInstance.m_materialIdentifier = (uint32)MATERIAL_TYPE::MODEL3D;
		m_default3DMaterialsInstanceValidationData.MarkResourceAsDirty(0);
		for (size_t frameInFlight = 0; frameInFlight < MAX_FRAMESINFLIGHT; frameInFlight++)
		{
			if (!InitMaterialInstanceInternal(m_defaultSpriteMaterialInstance, frameInFlight, true))
			{
				//Assert here
			}
			if (!InitMaterialInstanceInternal(m_default3DMaterialInstance, frameInFlight, true))
			{
				//Assert here
			}
		}

	}

	//TODO: Add relative path support in the shader output
	CompiledShader MaterialManager::LoadShader(const char* shaderName, SHADERTYPE shaderType, bool reloadShader)
	{
		CompiledShader shader{};
		String256 inPath = String256::Format("%s%s%s", SHADER_DIR_OUT, shaderName, ".shr");

		InOutStream inStream;

		if (!inStream.OpenFile(inPath.Data(), FILE_OPEN_TYPE::READ, true) || reloadShader)
		{
			InitCompiler();
			if (!m_compiler->CompileSpecificShader(shaderName, shaderType))
			{
				return shader;
			}
			DeInitCompiler();
		}
		inStream.OpenFile(inPath.Data(), FILE_OPEN_TYPE::READ, true);

		//Debug_PrintConsoleString256(String256::Format("\nImporting Shader:\n%s:", shaderName));

		inStream.Read((char*)&shader.header, sizeof(ShaderHeader));
		//Debug_PrintConsoleString256(String256::Format("Shader Size:%i:%s", shader.header.sizeOfShaderData, "\n"));

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

	ResourceValidator& MaterialManager::GetDefaultMaterialValidator(MATERIAL_TYPE type)
	{
		if (type == MATERIAL_TYPE::SPRITE)
			return m_defaultSpriteMaterialsInstanceValidationData;
		if (type == MATERIAL_TYPE::MODEL3D)
			return m_default3DMaterialsInstanceValidationData;
	}

	void MaterialManager::ClearAllResources()
	{
	}

	FilePath MaterialManager::CreateMaterial(const FilePath& outPath, const String256& name) const
	{
		FilePath finalPath = outPath + FileObject(name.Data(), "mat", outPath);
		InOutStream outStream;
		outStream.OpenFile(finalPath, FILE_OPEN_TYPE::WRITE, true);

		SerializeableMaterialInstance resource{};
		outStream.Write(&resource, sizeof(SerializeableMaterialInstance));
		outStream.CloseFile();
		outStream.OpenFile(finalPath, FILE_OPEN_TYPE::APPENDS, true);
		finalPath.LoadCommonFileData();
		
		MetaResource textureMetaResource;
		textureMetaResource.ConstructResourceAndID(finalPath, finalPath);
		textureMetaResource.Serialize(outStream);
		GetResourceRegistry().AddToRegistry(finalPath, ResourceType::Material);

		return finalPath;
	}

	void MaterialManager::ExportMaterial(MaterialResourceContextObject& materialToExport)
	{
		FilePath finalPath = materialToExport.m_metaResource.GetProjectFilePath().GetFilePath();
		InOutStream outStream;
		outStream.OpenFile(finalPath, FILE_OPEN_TYPE::WRITE, true);

		outStream.Write(&materialToExport.m_materialObject, sizeof(SerializeableMaterialInstance));
		outStream.CloseFile();
		outStream.OpenFile(finalPath, FILE_OPEN_TYPE::APPENDS, true);
		finalPath.LoadCommonFileData();
		materialToExport.m_metaResource.Serialize(outStream);
	}

	void MaterialManager::LoadMaterialMetaData(const FilePath& materialPath, MetaResource& resourceToFill)
	{
		if (!StringCompare(materialPath.Object().Extension(), L"mat"))
			return;

		InOutStream inStream;
		if (!inStream.OpenFile(materialPath.Data(), FILE_OPEN_TYPE::READ, true))
			return;

		TextureResource textureToFill;
		inStream.Seek(sizeof(SerializeableMaterialInstance), 1);

		const uint64 sizeLeft = inStream.GetFileSize() - inStream.GetFileSeekPosition();
		if (sizeLeft != 0)
		{
			resourceToFill.Deserialize(inStream);
		}
	}
	SerializeableMaterialInstance MaterialManager::LoadMaterialSerializeableInstance(const FilePath& materialPath)
	{
		SerializeableMaterialInstance returnResource;
		if (!StringCompare(materialPath.Object().Extension(), L"mat"))
			return returnResource;

		InOutStream inStream;
		if (!inStream.OpenFile(materialPath.Data(), FILE_OPEN_TYPE::READ, true))
			return returnResource;

		inStream.Read(&returnResource, sizeof(SerializeableMaterialInstance));
		return returnResource;
	}
}
