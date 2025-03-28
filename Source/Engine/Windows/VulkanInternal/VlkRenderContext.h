#pragma once

#include "Rendering\RenderContext.h"
#include "VlkDevice.h"

namespace Hail
{
	class VlkBufferObject;

	class VlkCommandBuffer : public CommandBuffer
	{
	public:
		VlkCommandBuffer(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer);
		
		friend class VlkRenderContext;
		friend class VlkRenderer;

	private:
		void BeginBufferInternal() override;
		void EndBufferInternal(bool bDestroyBufferData) override;
		VkCommandBuffer m_commandBuffer;
	};

	class VlkRenderContext : public RenderContext
	{
	public:
		explicit VlkRenderContext(RenderingDevice* device, ResourceManager* pResourceManager);

		void Cleanup() override;

		void BindMaterialInstance(uint32 materialInstanceIndex) override;

		CommandBuffer* CreateCommandBufferInternal(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer) override;
		void UploadDataToBufferInternal(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData) override;
		void UploadDataToTextureInternal(TextureResource* pTexture, void* pDataToUpload, uint32 mipLevel);
		void TransferFramebufferLayoutInternal(TextureResource* pTextureToTransfer, eFrameBufferLayoutState sourceState, eFrameBufferLayoutState destinationState) override;
		void RenderMeshlets(glm::uvec3 dispatchSize) override;
		void RenderFullscreenPass() override;
		void RenderSprites(uint32 numberOfInstances, uint32 offset) override;
		bool BindMaterialInternal(Pipeline* pPipeline) override;
		void ClearFrameBufferInternal(FrameBufferTexture* pFrameBuffer) override;

		void SetPushConstantInternal(void* pPushConstant) override;

		void StartFrame() override;
		void EndRenderPass() override;
		void SubmitFinalFrameCommandBuffer() override;
	private:
		void BindMaterialFrameBufferConnection(MaterialFrameBufferConnection* connectionToBind) override;
		void BindVertexBufferInternal() override;
		MaterialFrameBufferConnection* CreateMaterialFrameBufferConnection() override;
		VlkBufferObject* CreateStagingBufferAndMemoryBarrier(uint32 bufferSize, void* pDataToUpload);

		class VlkMaterialFrameBufferConnection : public MaterialFrameBufferConnection
		{
		public:
			void Cleanup(RenderingDevice* pDevice) override;

			VkPipeline m_pipeline = VK_NULL_HANDLE;
			VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		};

		bool CreateGraphicsPipeline(VlkMaterialFrameBufferConnection& materialFrameBufferConnection);

		VkSemaphore m_imageAvailableSemaphores[MAX_FRAMESINFLIGHT];
		VkSemaphore m_renderFinishedSemaphores[MAX_FRAMESINFLIGHT];
		VkFence m_inFrameFences[MAX_FRAMESINFLIGHT];
	};

}
