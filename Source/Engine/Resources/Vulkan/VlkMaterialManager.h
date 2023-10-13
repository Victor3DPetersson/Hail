#pragma once
#include "Resources\MaterialManager.h"
#include "Types.h"
#include "Windows\VulkanInternal\VlkResources.h"

namespace Hail
{
	class VlkFrameBufferTexture;

	class VlkMaterialManager : public MaterialManager
	{
	public:

		void Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain) final;

		void ClearAllResources() final;

		VlkPassData& GetMaterialData(MATERIAL_TYPE material);

	private:

		bool InitMaterialInternal(MATERIAL_TYPE materialType, FrameBufferTexture* frameBufferToBindToMaterial) final;
		bool InitMaterialInstanceInternal(const Material material, MaterialInstance& instance) final;
		bool CreateMaterialPipeline(Material& material);
		bool SetUpMaterialLayouts(VlkPassData& passData, MATERIAL_TYPE type);
		bool CreateRenderpassAndFramebuffers(VlkPassData& passData, MATERIAL_TYPE type);

		VlkFrameBufferTexture* m_passesFrameBufferTextures[(uint32)(MATERIAL_TYPE::COUNT)];
		VlkPassData m_passData[(uint32)(MATERIAL_TYPE::COUNT)];

		//This is here so the local variables does not get optimized away in release
		VkDescriptorImageInfo m_descriptorImageInfo;
		VkDescriptorBufferInfo m_descriptorBufferInfo;
	};

}