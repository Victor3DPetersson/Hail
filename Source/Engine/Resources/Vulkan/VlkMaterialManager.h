#pragma once
#include "Resources\MaterialManager.h"
#include "Windows\VulkanInternal\VlkResources.h"
#include "Types.h"

namespace Hail
{
	class VlkFrameBufferTexture;
	class VlkPassData;

	class VlkMaterialManager : public MaterialManager
	{
	public:

		void Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain) final;

		void ClearAllResources() final;

		VlkPassData& GetMaterialData(eMaterialType material, uint32 materialIndex);

	private:

		void BindFrameBuffer(eMaterialType materialType, FrameBufferTexture* frameBufferToBindToMaterial) final;
		bool InitMaterialInternal(Material& material, uint32 frameInFlight) final;
		bool InitMaterialInstanceInternal(MaterialInstance& instance, uint32 frameInFlight, bool isDefaultMaterialInstance) final;
		void ClearMaterialInternal(Material* pMaterial, uint32 frameInFlight) final;
		Material* CreateUnderlyingMaterial() final;
		bool CreateMaterialPipeline(Material& material, uint32 frameInFlight);
		bool SetUpMaterialLayouts(VlkPassData& passData, ResourceValidator& passDataValidator, eMaterialType type, uint32 frameInFlight);
		bool SetUpPipelineLayout(VlkPassData& passData, eMaterialType type, uint32 frameInFlight);
		bool CreateFramebuffers(VlkPassData& passData, eMaterialType type, uint32 frameInFlight);
		bool CreateRenderpass(VlkPassData& passData, eMaterialType type);


		VlkFrameBufferTexture* m_passesFrameBufferTextures[(uint32)(eMaterialType::COUNT)];


		//This is here so the local variables does not get optimized away in release
		VkDescriptorImageInfo m_descriptorImageInfo;
	};

}