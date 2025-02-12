#pragma once
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"
#include "Resources_Materials/ShaderCommons.h"
#include "MaterialResources.h"


namespace Hail
{
	class FilePath;
	class FrameBufferTexture;
	class MetaResource;
	class RenderContext;
	class RenderingDevice;
	class RenderingResourceManager;
	class ShaderCompiler;
	class SwapChain;
	class TextureManager;

	struct MaterialResourceContextObject;

	class MaterialManager
	{
	public:

		virtual void Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain);
		
		void Update();

		// Loads the base default materials for each type.
		bool InitDefaultMaterial(eMaterialType  type, FrameBufferTexture* frameBufferToBindToMaterial, bool reloadShader, uint32 frameInFlight);
		Material* GetMaterial(eMaterialType materialType, uint32 materialIndex);
		Material* GetDefaultMaterial(eMaterialType materialType);

		MaterialPipeline* CreateMaterialPipeline(MaterialCreationProperties props);

		const MaterialInstance& GetMaterialInstance(uint32_t instanceID, eMaterialType materialType);
		// Creates a material Instance from a materialType and materialSortKey which corresponds to a Shader Blend mode Key.
		bool InitMaterialInstance(eMaterialType materialType, MaterialInstance instanceData);
		uint32 GetMaterialInstanceHandle(GUID guid) const;

		bool InitMaterialInstance(uint32 instanceID, uint32 frameInFlight);
		bool ReloadAllMaterials(uint32 frameInFlight);
		bool ReloadAllMaterialInstances(uint32 frameInFlight);
		void ClearAllResources();

		void InitDefaultMaterialInstances();
		
		bool LoadMaterialFromSerializeableInstanceGUID(const GUID guid);
		bool StaggeredMaterialLoad(const GUID guid);

		MaterialTypeObject* GetTypeData(Pipeline* pPipeline);

		virtual void BindPipelineToContext(Pipeline* pPipeline, RenderContext* pRenderContext) = 0;

		// Resource handling functionality
		FilePath CreateMaterial(const FilePath& outPath, const String64& name, eMaterialType type) const;
		void ExportMaterial(MaterialResourceContextObject& materialToExport);
		static void LoadMaterialMetaData(const FilePath& materialPath, MetaResource& resourceToFill);
		static SerializeableMaterial LoadMaterialSerializeableInstance(const FilePath& materialPath);
		static MetaResource LoadShaderMetaData(const FilePath& shaderPath);
		CompiledShader* GetDefaultCompiledLoadedShader(eShaderType shaderType);
		CompiledShader* GetCompiledLoadedShader(GUID shaderGUID);
		CompiledShader* LoadShaderResource(GUID shaderGUID);
		FilePath ImportShaderResource(const FilePath& filepath);

		const bool IsShaderValidWithMaterialType(eMaterialType materialType, eShaderType shaderType, ReflectedShaderData& shaderDataToCheck) const;

	protected:

		// Returns the index of the loaded material, will be MAX_UINT if not loaded.
		// Also loads and imports the shaders.
		uint32 LoadMaterialFromSerializedData(const SerializeableMaterial& loadedMaterial);
		virtual void BindFrameBuffer(eMaterialType materialType, FrameBufferTexture* frameBufferToBindToMaterial) = 0;
		virtual bool InitMaterialInternal(Material* pMaterial, uint32 frameInFlight) = 0;
		virtual bool InitMaterialPipelineInternal(MaterialPipeline* pMaterial, uint32 frameInFlight) = 0;
		virtual bool InitMaterialInstanceInternal(MaterialInstance& instance, uint32 frameInFlight, bool isDefaultMaterialInstance) = 0;
		virtual void ClearMaterialInternal(Material* pMaterial, uint32 frameInFlight) = 0;
		// Creates the material that is based on the underlying API
		virtual Material* CreateUnderlyingMaterial() = 0;
		virtual MaterialPipeline* CreateUnderlyingMaterialPipeline() = 0;
		virtual Pipeline* CreateUnderlyingPipeline() = 0;
		// Setting up the material type, or binds the already created type definition to the material
		virtual bool CreateMaterialTypeObject(Pipeline* pPipelinel) = 0;

		void ClearHighLevelMaterial(Material* pMaterial, uint32 frameInFlight);
		void CheckMaterialInstancesToReload(uint32 frameInFlight);


		CompiledShader* LoadShader(const char* shaderName, eShaderType shaderType, bool reloadShader, const char* shaderExtension = "");
		void InitCompiler();
		void DeInitCompiler();

		ResourceValidator& GetDefaultMaterialValidator(eMaterialType type);
		const ReflectedShaderData* GetDefaultShaderData(eMaterialType materialType, eShaderType shaderType) const;

		// Resource dependencies
		RenderingDevice* m_renderDevice = nullptr;
		RenderingResourceManager* m_renderingResourceManager = nullptr;
		TextureManager* m_textureManager = nullptr;
		SwapChain* m_swapChain = nullptr;

		ShaderCompiler* m_compiler = nullptr;

		StaticArray<GrowingArray<Material*>, (uint32)eMaterialType::COUNT> m_materials;
		StaticArray<MaterialTypeObject*, (uint32)eMaterialType::COUNT> m_MaterialTypeObjects;

		// TODO: maybe join these together in to one object
		GrowingArray<MaterialInstance> m_materialsInstanceData;
		GrowingArray<ResourceValidator> m_materialsInstanceValidationData;
		
		struct MaterialLoadRequest
		{
			StaticArray<GUID, MAX_TEXTURE_HANDLES> textureHandles;
			MaterialInstance materialInstance;
			uint32 numberOfTexturesInMaterial;
		};
		GrowingArray<MaterialLoadRequest> m_loadRequests;

		MaterialInstance m_defaultSpriteMaterialInstance;
		ResourceValidator m_defaultSpriteMaterialsInstanceValidationData;

		MaterialInstance m_default3DMaterialInstance;
		ResourceValidator m_default3DMaterialsInstanceValidationData;

		// TODO: better data structure
		StaticArray<GrowingArray<CompiledShader>, (uint32)eShaderType::None> m_loadedShaders;
		StaticArray<VectorOnStack<CompiledShader*, 2>, (uint32)eMaterialType::COUNT> m_defaultShaders;
	};
}
