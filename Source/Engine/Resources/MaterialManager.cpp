#include "Engine_PCH.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "ShaderCompiler.h"

#include "DebugMacros.h"
#include "Utility\InOutStream.h"
#include "HailEngine.h"
#include "ResourceRegistry.h"

#include "ImGui\ImGuiContext.h"
#include "Hashing\xxh64_en.hpp"

namespace Hail
{

	void MaterialManager::Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain)
	{
		m_renderDevice = renderingDevice;
		m_textureManager = textureResourceManager;
		m_swapChain = swapChain;
		m_renderingResourceManager = renderingResourceManager;
	}

	uint32 TempLocalGetShaderHash(eMaterialType type)
	{
		uint32 vertexShaderHash = 0;
		uint32 pixelShaderHash = 0;
		switch (type)
		{
		case eMaterialType::SPRITE:
			vertexShaderHash = xxh64::hash("VS_Sprite", StringLength("VS_Sprite"), 1337);
			pixelShaderHash = xxh64::hash("FS_Sprite", StringLength("FS_Sprite"), 1337);
			break;
		case eMaterialType::MODEL3D:
			vertexShaderHash = xxh64::hash("VS_triangle", StringLength("VS_triangle"), 1337);
			pixelShaderHash = xxh64::hash("FS_triangle", StringLength("FS_triangle"), 1337);
			break;
		case eMaterialType::FULLSCREEN_PRESENT_LETTERBOX:
			vertexShaderHash = xxh64::hash("VS_fullscreenPass", StringLength("VS_fullscreenPass"), 1337);
			pixelShaderHash = xxh64::hash("FS_fullscreenPass", StringLength("FS_fullscreenPass"), 1337);
			break;
		case eMaterialType::DEBUG_LINES2D:
		case eMaterialType::DEBUG_LINES3D:
			vertexShaderHash = xxh64::hash("VS_DebugLines", StringLength("VS_DebugLines"), 1337);
			pixelShaderHash = xxh64::hash("FS_DebugLines", StringLength("FS_DebugLines"), 1337);
			break;
		case eMaterialType::COUNT:
			break;
		default:
			break;
		}
		return pixelShaderHash + vertexShaderHash;
	}

	bool MaterialManager::InitDefaultMaterial(eMaterialType type, FrameBufferTexture* frameBufferToBindToMaterial, bool reloadShader, uint32 frameInFlight)
	{
		if (m_materials[(uint32_t)type].Empty())
		{

			Material* pMaterial = CreateUnderlyingMaterial();
			switch (type)
			{
			case eMaterialType::SPRITE:
				pMaterial->m_vertexShader = LoadShader("VS_Sprite", SHADERTYPE::VERTEX, reloadShader);
				pMaterial->m_fragmentShader = LoadShader("FS_Sprite", SHADERTYPE::FRAGMENT, reloadShader);
				pMaterial->m_type = eMaterialType::SPRITE;
				break;
			case eMaterialType::FULLSCREEN_PRESENT_LETTERBOX:
				pMaterial->m_vertexShader = LoadShader("VS_fullscreenPass", SHADERTYPE::VERTEX, reloadShader);
				pMaterial->m_fragmentShader = LoadShader("FS_fullscreenPass", SHADERTYPE::FRAGMENT, reloadShader);
				pMaterial->m_type = eMaterialType::FULLSCREEN_PRESENT_LETTERBOX;
				break;
			case eMaterialType::MODEL3D:
				pMaterial->m_vertexShader = LoadShader("VS_triangle", SHADERTYPE::VERTEX, reloadShader);
				pMaterial->m_fragmentShader = LoadShader("FS_triangle", SHADERTYPE::FRAGMENT, reloadShader);
				pMaterial->m_type = eMaterialType::MODEL3D;
				break;
			case eMaterialType::DEBUG_LINES2D:
				pMaterial->m_vertexShader = LoadShader("VS_DebugLines", SHADERTYPE::VERTEX, reloadShader);
				pMaterial->m_fragmentShader = LoadShader("FS_DebugLines", SHADERTYPE::FRAGMENT, reloadShader);
				pMaterial->m_type = eMaterialType::DEBUG_LINES2D;
				break;
			case eMaterialType::DEBUG_LINES3D:
				pMaterial->m_vertexShader = LoadShader("VS_DebugLines", SHADERTYPE::VERTEX, reloadShader);
				pMaterial->m_fragmentShader = LoadShader("FS_DebugLines", SHADERTYPE::FRAGMENT, reloadShader);
				pMaterial->m_type = eMaterialType::DEBUG_LINES3D;
				break;
			case eMaterialType::COUNT:
				break;
			default:
				break;
			}
			if (pMaterial->m_vertexShader.loadState != SHADER_LOADSTATE::LOADED_TO_RAM ||
				pMaterial->m_fragmentShader.loadState != SHADER_LOADSTATE::LOADED_TO_RAM)
			{
				return false;
			}
			uint32 combinedShaderHash = TempLocalGetShaderHash(pMaterial->m_type);
			const uint64 materialSortKey = GetMaterialSortValue(pMaterial->m_type, pMaterial->m_blendMode, combinedShaderHash);
			pMaterial->m_sortKey = type != eMaterialType::DEBUG_LINES3D ? materialSortKey : materialSortKey + 1u;
			pMaterial->m_type = type;
			m_materials[(uint32_t)type].Add(pMaterial);
		}

		BindFrameBuffer(type, frameBufferToBindToMaterial);

		if (InitMaterialInternal(*m_materials[(uint32_t)type].GetLast(), frameInFlight))
		{
			m_materials[(uint32_t)type][0]->m_validator.ClearFrameData(frameInFlight);
			return true;
		}
		return false;
	}



	bool MaterialManager::LoadMaterialFromInstance(const SerializeableMaterialInstance& loadedMaterialInstance)
	{
		// TODO: add shaders to material instance so this will be data driven instead of hard-coded
		uint32 combinedShaderHash = TempLocalGetShaderHash(loadedMaterialInstance.m_baseMaterialType);
		const uint64 materialSortKey = GetMaterialSortValue(loadedMaterialInstance.m_baseMaterialType, loadedMaterialInstance.m_blendMode, combinedShaderHash);
		for (size_t i = 0; i < m_materials[(uint8)loadedMaterialInstance.m_baseMaterialType].Size(); i++)
		{
			Material& loadedMaterial = *m_materials[(uint8)loadedMaterialInstance.m_baseMaterialType][i];
			if (loadedMaterial.m_sortKey == materialSortKey)
				return true;
		}

		Material* pMaterial = CreateUnderlyingMaterial();
		pMaterial->m_sortKey = materialSortKey;
		pMaterial->m_type = loadedMaterialInstance.m_baseMaterialType;
		pMaterial->m_blendMode = loadedMaterialInstance.m_blendMode;
		// Todo: The shaders should be saved in on the material instance and make the shaders be cached and handled more smartly
		switch (loadedMaterialInstance.m_baseMaterialType)
		{
		case eMaterialType::SPRITE:
			pMaterial->m_vertexShader = LoadShader("VS_Sprite", SHADERTYPE::VERTEX, false);
			pMaterial->m_fragmentShader = LoadShader("FS_Sprite", SHADERTYPE::FRAGMENT, false);
			break;
		case eMaterialType::MODEL3D:
			pMaterial->m_vertexShader = LoadShader("VS_triangle", SHADERTYPE::VERTEX, false);
			pMaterial->m_fragmentShader = LoadShader("FS_triangle", SHADERTYPE::FRAGMENT, false);
			break;
		case eMaterialType::FULLSCREEN_PRESENT_LETTERBOX:
		case eMaterialType::DEBUG_LINES2D:
		case eMaterialType::DEBUG_LINES3D:
			
			break;
		case eMaterialType::COUNT:
			break;
		default:
			break;
		}
		bool result = true;
		for (size_t i = 0; i < MAX_FRAMESINFLIGHT; i++)
		{
			if (InitMaterialInternal(*pMaterial, i))
			{
				pMaterial->m_validator.ClearFrameData(i);
			}
			else
			{
				result &= false;
			}
		}

		if (!result)
		{

			// TODO assert / error
		}
		m_materials[(uint32_t)loadedMaterialInstance.m_baseMaterialType].Add(pMaterial);

		return result;
	}

	Material& MaterialManager::GetMaterial(eMaterialType materialType, uint32 materialIndex)
	{
		// TODO add bound checks
		return *m_materials[(uint32)materialType][materialIndex];
	}

	const MaterialInstance& MaterialManager::GetMaterialInstance(uint32_t instanceID, eMaterialType materialType)
	{
		if (instanceID >= m_materialsInstanceData.Size())
		{
			if (materialType == eMaterialType::SPRITE)
				return m_defaultSpriteMaterialInstance;
			if (materialType == eMaterialType::MODEL3D)
				return m_default3DMaterialInstance;
		}

		return m_materialsInstanceData[instanceID];
	}

	uint32 MaterialManager::CreateInstance(eMaterialType materialType, MaterialInstance instanceData)
	{
		instanceData.m_instanceIdentifier = m_materialsInstanceData.Size();
		//TODO: Get hash from serialized shader values instead
		const uint64 materialSortKey = GetMaterialSortValue(materialType, instanceData.m_blendMode, TempLocalGetShaderHash(materialType));
		bool isMaterialLoaded = false;
		uint32 materialIndex = MAX_UINT;
		for (size_t i = 0; i < m_materials[(uint8)materialType].Size(); i++)
		{
			Material& loadedMaterial = *m_materials[(uint8)materialType][i];
			if (loadedMaterial.m_sortKey == materialSortKey)
			{
				isMaterialLoaded = true;
				materialIndex = i;
			}
		}
		if (!isMaterialLoaded)
		{
			// TODO: Assert here as a material should always be loaded before creating an instance
			instanceData.m_materialIndex = 0;
		}
		instanceData.m_materialIndex = materialIndex;
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
		for (uint32 i = 0; i < (uint32)(eMaterialType::COUNT); i++)
		{
			for (uint32 iMaterial = 0; iMaterial < m_materials[i].Size(); iMaterial++)
			{
				// TODO assert on the pointer here 
				const bool firstFrameOfReload = m_materials[i][iMaterial]->m_validator.GetIsResourceDirty() == false;
				if (firstFrameOfReload)
				{
					ClearHighLevelMaterial(m_materials[i][iMaterial], frameInFlight);
				}
				ClearMaterialInternal(m_materials[i][iMaterial], frameInFlight);
			}
			if (!InitDefaultMaterial((eMaterialType)i, nullptr, true, frameInFlight))
			{
				// TODO add assert
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

	void MaterialManager::ClearHighLevelMaterial(Material* pMaterial, uint32 frameInFlight)
	{
		//TODO make more proper and robust validation later =) 
		pMaterial->m_fragmentShader.loadState = SHADER_LOADSTATE::UNLOADED;
		pMaterial->m_vertexShader.loadState = SHADER_LOADSTATE::UNLOADED;
		pMaterial->m_validator.MarkResourceAsDirty(frameInFlight);
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
		m_defaultSpriteMaterialInstance.m_materialIndex = 0;// GetMaterialSortValue(eMaterialType::SPRITE, eBlendMode::None, TempLocalGetShaderHash(eMaterialType::SPRITE));
		m_defaultSpriteMaterialsInstanceValidationData.MarkResourceAsDirty(0);

		m_default3DMaterialInstance.m_instanceIdentifier = MAX_UINT;
		m_default3DMaterialInstance.m_materialIndex = 0;// GetMaterialSortValue(eMaterialType::MODEL3D, eBlendMode::None, TempLocalGetShaderHash(eMaterialType::MODEL3D));
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

	ResourceValidator& MaterialManager::GetDefaultMaterialValidator(eMaterialType type)
	{
		if (type == eMaterialType::SPRITE)
			return m_defaultSpriteMaterialsInstanceValidationData;
		if (type == eMaterialType::MODEL3D)
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
