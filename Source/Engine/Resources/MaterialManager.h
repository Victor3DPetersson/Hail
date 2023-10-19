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

	class MaterialManager
	{
	public:

		virtual void Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain);
		bool InitMaterial(MATERIAL_TYPE  type, FrameBufferTexture* frameBufferToBindToMaterial, bool reloadShader, uint32 frameInFlight);
		bool LoadMaterial(GUID uuid);
		Material& GetMaterial(MATERIAL_TYPE materialType);
		const MaterialInstance& GetMaterialInstance(uint32_t instanceID);
		uint32 CreateInstance(MATERIAL_TYPE materialType, MaterialInstance instanceData);
		bool InitMaterialInstance(uint32 instanceID, uint32 frameInFlight);
		virtual bool ReloadAllMaterials(uint32 frameInFlight);
		virtual void ClearAllResources();

	protected:

		virtual bool InitMaterialInternal(MATERIAL_TYPE materialType, FrameBufferTexture* frameBufferToBindToMaterial, uint32 frameInFlight) = 0;
		virtual bool InitMaterialInstanceInternal(MaterialInstance& instance, uint32 frameInFlight) = 0;
		virtual void ClearMaterialInternal(MATERIAL_TYPE materialType, uint32 frameInFlight) = 0;

		void ClearHighLevelMaterial(MATERIAL_TYPE materialType, uint32 frameInFlight);

		CompiledShader LoadShader(const char* shaderName, SHADERTYPE shaderType, bool reloadShader);
		void InitCompiler();
		void DeInitCompiler();

		RenderingDevice* m_renderDevice = nullptr;
		RenderingResourceManager* m_renderingResourceManager = nullptr;
		TextureManager* m_textureManager = nullptr;
		SwapChain* m_swapChain = nullptr;

		ShaderCompiler* m_compiler = nullptr;
		GrowingArray<CompiledShader> m_compiledRequiredShaders;
		Material m_materials[(uint32)MATERIAL_TYPE::COUNT];
		ResourceValidator m_materialValidators[(uint32)MATERIAL_TYPE::COUNT];

		GrowingArray<MaterialInstance> m_materialsInstanceData;
		GrowingArray<ResourceValidator> m_materialsInstanceValidationData;
	};
}
