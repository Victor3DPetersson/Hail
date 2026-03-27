#pragma once

#include "Rendering\RenderContext.h"
#include "VlkDevice.h"

namespace Hail
{
	class VlkBufferObject;

	class VlkCommandBuffer : public CommandBuffer
	{
	public:
		VlkCommandBuffer(RenderingDevice* pDevice, VkCommandPool* pVkCommandPool);
		
		void Cleanup(RenderingDevice* pDevice, uint32 frame) override;
		friend class VlkRenderContext;
		friend class VlkRenderer;

	private:
		void BeginBufferInternal() override;
		void EndBufferInternal() override;
		VkCommandBuffer m_commandBuffer;
		VkSemaphore m_semaphore;
	};

	class VlkRenderContext : public RenderContext
	{
	public:
		explicit VlkRenderContext(RenderContextStartupParams renderContextStartParams);

		void Cleanup() override;

		void BindMaterialInstance(uint32 materialInstanceIndex) override;

		void UploadDataToBufferInternal(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData) override;
		void CopyDataToBufferInternal(BufferObject* pDstBuffer, BufferObject* pSrcBuffer) override;
		void UploadDataToTextureInternal(TextureResource* pTexture, void* pDataToUpload, uint32 mipLevel);
		void TransferFramebufferLayoutInternal(TextureResource* pTextureToTransfer, eFrameBufferLayoutState sourceState, eFrameBufferLayoutState destinationState) override;
		void TransferImageStateInternal(TextureResource* pTexture, eShaderAccessQualifier newState, uint32 newStageCombination) override;

		void Dispatch(glm::uvec3 dispatchSize) override;
		void RenderMeshlets(glm::uvec3 dispatchSize) override;
		void RenderFullscreenPass() override;
		void RenderInstances(uint32 numberOfInstances, uint32 offset) override;
		void RenderDebugLines(uint32 numberOfLinesToRender) override;

		bool BindMaterialInternal(Pipeline* pPipeline) override;
		bool BindComputePipelineInternal(Pipeline* pPipeline) override;
		void ClearFrameBufferInternal(FrameBufferTexture* pFrameBuffer) override;

		void SetPushConstantInternal(void* pPushConstant) override;

		void StartFrame() override;
		void EndCurrentPass(uint32 nextShaderStage) override;
		void SubmitFinalFrameCommandBuffer() override;

		void TransferBufferStateInternal(BufferObject* pBuffer, eShaderAccessQualifier newState) override;
	private:
		void BindMaterialFrameBufferConnection(MaterialFrameBufferConnection* connectionToBind) override;
		void BindComputePipeline(ComputePipeline* pPipelineToBind) override;
		void BindVertexBufferInternal() override;
		VlkBufferObject* CreateStagingBufferAndMemoryBarrier(uint32 bufferSize, void* pDataToUpload);

		class VlkMaterialFrameBufferConnection : public MaterialFrameBufferConnection
		{
		public:
			void Cleanup(RenderingDevice* pDevice) override;

			VkPipeline m_pipeline = VK_NULL_HANDLE;
			VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		};

		class VlkComputePipeline : public ComputePipeline
		{
		public:
			void Cleanup(RenderingDevice* pDevice) override;
			VkPipeline m_pipeline = VK_NULL_HANDLE;
			VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		};

		bool CreateGraphicsPipeline(VlkMaterialFrameBufferConnection& materialFrameBufferConnection);
		bool CreateComputePipeline(VlkComputePipeline& computePipeline);

		class VkFrameData : public FrameCommandData
		{
		public:
			void Init(RenderingDevice* pDevice, uint32 frame) override;
			void CommitCommandBuffer(RenderingDevice* pDevice) override;
			void Cleanup(RenderingDevice* pDevice, uint32 frame) override;
			void Reset(RenderingDevice* pDevice) override;
			VkCommandPool* m_pCommandPool;

			VkSemaphore m_imageAvailable;
			VkSemaphore m_renderFinished;

			VkFence m_inFlightFence;
		};
	};

}
