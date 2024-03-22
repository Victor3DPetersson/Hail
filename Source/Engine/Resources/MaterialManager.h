#pragma once
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"
#include "ShaderCommons.h"
#include "MaterialResources.h"


namespace Hail
{
	class ShaderCompiler;
	class FrameBufferTexture;
	class RenderingDevice;
	class TextureManager;
	class RenderingResourceManager;
	class SwapChain;
	class FilePath;
	class MetaResource;
	struct MaterialResourceContextObject;

	class MaterialManager
	{
	public:

		virtual void Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain);
		// Loads the base default materials for each type
		bool InitDefaultMaterial(eMaterialType  type, FrameBufferTexture* frameBufferToBindToMaterial, bool reloadShader, uint32 frameInFlight);
		bool LoadMaterialFromInstance(const SerializeableMaterialInstance& loadedMaterial);
		Material& GetMaterial(eMaterialType materialType, uint32 materialIndex);
		const MaterialInstance& GetMaterialInstance(uint32_t instanceID, eMaterialType materialType);
		uint32 CreateInstance(eMaterialType materialType, MaterialInstance instanceData);
		uint32 GetMaterialInstanceHandle(GUID guid) const;

		bool InitMaterialInstance(uint32 instanceID, uint32 frameInFlight);
		bool ReloadAllMaterials(uint32 frameInFlight);
		bool ReloadAllMaterialInstances(uint32 frameInFlight);
		virtual void ClearAllResources();

		void InitDefaultMaterialInstances();

		//Editor / non game functionality.
		FilePath CreateMaterial(const FilePath& outPath, const String256& name) const;
		void ExportMaterial(MaterialResourceContextObject& materialToExport);
		static void LoadMaterialMetaData(const FilePath& materialPath, MetaResource& resourceToFill);
		static SerializeableMaterialInstance LoadMaterialSerializeableInstance(const FilePath& materialPath);

	protected:

		virtual void BindFrameBuffer(eMaterialType materialType, FrameBufferTexture* frameBufferToBindToMaterial) = 0;
		virtual bool InitMaterialInternal(Material& material, uint32 frameInFlight) = 0;
		virtual bool InitMaterialInstanceInternal(MaterialInstance& instance, uint32 frameInFlight, bool isDefaultMaterialInstance) = 0;
		virtual void ClearMaterialInternal(Material* pMaterial, uint32 frameInFlight) = 0;
		// Creates the material that is based on the underlying API
		virtual Material* CreateUnderlyingMaterial() = 0;


		void ClearHighLevelMaterial(Material* pMaterial, uint32 frameInFlight);
		void CheckMaterialInstancesToReload(uint32 frameInFlight);


		CompiledShader LoadShader(const char* shaderName, SHADERTYPE shaderType, bool reloadShader);
		void InitCompiler();
		void DeInitCompiler();

		ResourceValidator& GetDefaultMaterialValidator(eMaterialType type);

		RenderingDevice* m_renderDevice = nullptr;
		RenderingResourceManager* m_renderingResourceManager = nullptr;
		TextureManager* m_textureManager = nullptr;
		SwapChain* m_swapChain = nullptr;

		ShaderCompiler* m_compiler = nullptr;
		GrowingArray<CompiledShader> m_compiledRequiredShaders;

		StaticArray<GrowingArray<Material*>, (uint32)eMaterialType::COUNT> m_materials;

		GrowingArray<MaterialInstance> m_materialsInstanceData;
		

		MaterialInstance m_defaultSpriteMaterialInstance;
		ResourceValidator m_defaultSpriteMaterialsInstanceValidationData;

		MaterialInstance m_default3DMaterialInstance;
		ResourceValidator m_default3DMaterialsInstanceValidationData;

		GrowingArray<ResourceValidator> m_materialsInstanceValidationData;
	};
}
