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

		bool InitMaterialInternal(MATERIAL_TYPE materialType, FrameBufferTexture* frameBufferToBindToMaterial, uint32 frameInFlight) final;
		bool InitMaterialInstanceInternal(MaterialInstance& instance, uint32 frameInFlight) final;
		void ClearMaterialInternal(MATERIAL_TYPE materialType, uint32 frameInFlight) final;
		bool CreateMaterialPipeline(Material& material, uint32 frameInFlight);
		bool SetUpMaterialLayouts(VlkPassData& passData, MATERIAL_TYPE type, uint32 frameInFlight);
		bool SetUpPipelineLayout(VlkPassData& passData, MATERIAL_TYPE type, uint32 frameInFlight);
		bool CreateRenderpassAndFramebuffers(VlkPassData& passData, MATERIAL_TYPE type, uint32 frameInFlight);


		VlkFrameBufferTexture* m_passesFrameBufferTextures[(uint32)(MATERIAL_TYPE::COUNT)];
		VlkPassData m_passData[(uint32)(MATERIAL_TYPE::COUNT)];
		ResourceValidator m_passDataValidators[(uint32)MATERIAL_TYPE::COUNT];

		//This is here so the local variables does not get optimized away in release
		VkDescriptorImageInfo m_descriptorImageInfo;
		VkDescriptorBufferInfo m_descriptorBufferInfo;
	};

}