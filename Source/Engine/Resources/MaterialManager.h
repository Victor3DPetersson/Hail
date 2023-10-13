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

		void Update();
		virtual void Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain);
		bool InitMaterial(MATERIAL_TYPE  type, FrameBufferTexture* frameBufferToBindToMaterial);
		bool LoadMaterial(GUID uuid);
		Material& GetMaterial(MATERIAL_TYPE materialType);
		const MaterialInstance& GetMaterialInstance(uint32_t instanceID);
		bool CreateInstance(MATERIAL_TYPE materialType, MaterialInstance instanceData);
		virtual void ClearAllResources();

	protected:

		virtual bool InitMaterialInternal(MATERIAL_TYPE materialType, FrameBufferTexture* frameBufferToBindToMaterial) = 0;
		virtual bool InitMaterialInstanceInternal(const Material material, MaterialInstance& instance) = 0;

		CompiledShader LoadShader(const char* shaderName, SHADERTYPE shaderType);
		void InitCompiler();
		void DeInitCompiler();

		RenderingDevice* m_renderDevice = nullptr;
		RenderingResourceManager* m_renderingResourceManager = nullptr;
		TextureManager* m_textureManager = nullptr;
		SwapChain* m_swapChain = nullptr;

		ShaderCompiler* m_compiler = nullptr;
		GrowingArray<CompiledShader> m_compiledRequiredShaders;
		Material m_materials[static_cast<uint32_t>(MATERIAL_TYPE::COUNT)];
		GrowingArray< MaterialInstance> m_materialsInstanceData;
	};
}
