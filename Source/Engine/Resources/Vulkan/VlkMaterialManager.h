#pragma once
#include "Resources\MaterialManager.h"
#include "Windows\VulkanInternal\VlkResources.h"
#include "Types.h"

namespace Hail
{
	class VlkFrameBufferTexture;
	class VlkMaterial;

	class VlkMaterialManager : public MaterialManager
	{
	public:

		void Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain) override;

	private:

		void BindFrameBuffer(eMaterialType materialType, FrameBufferTexture* frameBufferToBindToMaterial) override;
		bool InitMaterialInternal(Material* pMaterial, uint32 frameInFlight) override;
		bool InitMaterialInstanceInternal(MaterialInstance& instance, uint32 frameInFlight, bool isDefaultMaterialInstance) override;
		void ClearMaterialInternal(Material* pMaterial, uint32 frameInFlight) override;
		Material* CreateUnderlyingMaterial() override;
		bool CreateFramebuffers(VlkMaterial& vlkMaterial, uint32 frameInFlight);
		// TODO: Fix this one to not be hard-coded when making a render-graph / setting up dependencies of materials.
		bool CreateRenderpass(VlkMaterial& vlkMaterial);
		bool CreateMaterialTypeDescriptor(Material* pMaterial) override;
		// The pipeline holds all the Sets layouts.
		bool CreatePipelineLayout(Material* pMaterial);
		bool CreateGraphicsPipeline(VlkMaterial& vlkMaterial);
		//bool CreateComputePipeline(VlkMaterial& vlkMaterial);

		// Assigns the buffers and samplers that are registered to the material, so not instance descriptors.
		void AllocateTypeDescriptors(VlkMaterial& vlkMaterial, uint32 frameInFlight);

		VlkFrameBufferTexture* m_passesFrameBufferTextures[(uint32)(eMaterialType::COUNT)];
	};

}