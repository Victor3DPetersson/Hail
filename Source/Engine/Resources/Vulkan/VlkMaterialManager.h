#pragma once
#include "Resources\MaterialManager.h"
#include "Windows\VulkanInternal\VlkResources.h"
#include "Types.h"

namespace Hail
{
	class VlkFrameBufferTexture;
	class VlkMaterial;
	class VlkMaterialTypeObject;
	class VlkPipeline;

	class VlkMaterialManager : public MaterialManager
	{
	public:

		void Init(RenderingDevice* renderingDevice, TextureManager* textureResourceManager, RenderingResourceManager* renderingResourceManager, SwapChain* swapChain) override;

	private:
		void UpdateCustomPipelineDescriptors(Pipeline* pPipeline, RenderContext* pRenderContext) override;

		void BindFrameBuffer(eMaterialType materialType, FrameBufferTexture* frameBufferToBindToMaterial) override;
		bool InitMaterialInternal(Material* pMaterial, uint32 frameInFlight) override;
		bool InitMaterialPipelineInternal(MaterialPipeline* pMaterialPipeline, uint32 frameInFlight) override;
		bool InitMaterialInstanceInternal(MaterialInstance& instance, uint32 frameInFlight, bool isDefaultMaterialInstance) override;
		void ClearMaterialInternal(Material* pMaterial, uint32 frameInFlight) override;
		Material* CreateUnderlyingMaterial() override;
		MaterialPipeline* CreateUnderlyingMaterialPipeline() override;
		Pipeline* CreateUnderlyingPipeline() override;
		bool CreateMaterialTypeObject(Pipeline* pPipeline) override;
		// The pipeline holds all the Sets layouts.
		bool CreatePipelineLayout(VlkPipeline& vlkPipeline, VlkMaterial* pMaterial);

		// TODO, remove function and make this go through the context
		// Assigns the buffers and samplers that are registered to the material, so not instance descriptors.
		void AllocateTypeDescriptors(VlkPipeline& vlkPipeline, uint32 frameInFlight);

		VlkFrameBufferTexture* m_passesFrameBufferTextures[(uint32)(eMaterialType::COUNT)];
	};

}