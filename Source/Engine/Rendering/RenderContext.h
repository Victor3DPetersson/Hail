#pragma once
#include "Types.h"

#include "Settings.h"
#include "ResourceCommon.h"
#include "Resources_Materials\Materials_Common.h"

#include "Resources_Textures\TextureCommons.h"
#include "Containers\StaticArray\StaticArray.h"
#include "Containers\GrowingArray\GrowingArray.h"

namespace Hail
{
	class BufferObject;
	class ErrorManager;
	class FrameBufferTexture;
	class Material;
	class MaterialManager;
	class ResourceManager;
	class RenderingDevice;
	class TextureResource;
	class TextureView;
	class Pipeline;

	struct SetDecoration;

	enum class eContextState
	{
		TransitionBetweenStates,
		Transfer, 
		Graphics,
		Compute
	};


	// Inherited class to get the currently used CommandBuffer
	class CommandBuffer
	{
	public:
		CommandBuffer(RenderingDevice* pDevice);
		~CommandBuffer();
		void BeginBuffer(eContextState contextStateForCommandBuffer);
		void EndBuffer();

		virtual void Cleanup(RenderingDevice* pDevice, uint32 frame) = 0;
	protected:
		virtual void BeginBufferInternal() = 0;
		virtual void EndBufferInternal() = 0;
		friend class RenderContext;
		eContextState m_contextState;
		const RenderingDevice* m_pDevice;
		bool m_bIsRecording;
	};

	struct RenderContextStartupParams
	{
		RenderingDevice* pDevice;
		ResourceManager* pResourceManager;
		ErrorManager* pErrorManager;
	};

	class RenderContext
	{
	public:
		RenderContext(RenderContextStartupParams renderContextStartParams);
		virtual void Cleanup() = 0;

		void SetBufferAtSlot(BufferObject* pBuffer, uint32 slot);
		void SetTextureAtSlot(TextureView* pTexture, uint32 slot);

		void BindMaterial(Material* pMaterial);
		void BindMaterial(Pipeline* pPipeline);
		// Will bind the instance ID for the currently bound material.
		virtual void BindMaterialInstance(uint32 materialInstanceIndex) = 0;
		void BindFrameBufferAtSlot(FrameBufferTexture* pFrameBuffer, uint32 bindSlot);
		void BindVertexBuffer(BufferObject* pVertexBufferToBind, BufferObject* pIndexBufferToBind);
		// Takes any 16 byte sized data
		void SetPushConstantValue(void* pPushConstant);

		// Clears the framebuffers colors to the base colors and depth values
		void ClearBoundFrameBuffers();

		// To clean up internal states and set them to invalid for validation and cleanup.
		virtual void DeleteFramebuffer(FrameBufferTexture* pFrameBufferToDelete);
		virtual void DeleteMaterial(Material* pMaterialToDelete);

		uint64 GetCurrentRenderFrame() const { return m_currentRenderFrame; }
		TextureView* GetBoundTextureAtSlot(uint32 slot);
		BufferObject* GetBoundStructuredBufferAtSlot(uint32 slot);
		BufferObject* GetBoundUniformBufferAtSlot(uint32 slot);

		void UploadDataToBuffer(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData);
		void UploadDataToTexture(TextureResource* pTexture, void* pDataToUpload, uint32 mipLevel);
		void CopyDataToBuffer(BufferObject* pDstBuffer, BufferObject* pSrcBuffer);

		void TransferFramebufferLayout(FrameBufferTexture* pTextureToTransfer, eFrameBufferLayoutState colorState, eFrameBufferLayoutState depthState);
		void TransferTextureLayout(TextureResource* pTextureToTransfer, eShaderAccessQualifier newQualifier, uint32 newStageCombination);
		void TransferBufferState(BufferObject* pBuffer, eShaderAccessQualifier newState);

		virtual void Dispatch(glm::uvec3 dispatchSize);
		virtual void RenderMeshlets(glm::uvec3 dispatchSize);
		virtual void RenderFullscreenPass() = 0;
		virtual void RenderInstances(uint32 numberOfInstances, uint32 offset) = 0;
		virtual void RenderDebugLines(uint32 numberOfLinesToRender) = 0;

		CommandBuffer* GetCurrentCommandBuffer() { return m_pCurrentCommandBuffer; }
		// 
		
		// Will end the frame if the last bound material that was bound is FULLSCREEN_PRESENT_LETTERBOX
		void StartGraphicsPass();
		void EndGraphicsPass();
		void StartTransferPass();
		void EndTransferPass();

		virtual void StartFrame() = 0;
		virtual void EndCurrentPass(uint32 nextShaderStage) = 0;

	protected:
		void CleanupAndEndPass();


		class MaterialFrameBufferConnection
		{
		public:
			virtual void Cleanup(RenderingDevice* pDevice) = 0;
			ResourceValidator m_validator;
			Pipeline* m_pMaterialPipeline = nullptr;
			FrameBufferTexture* m_pBoundFrameBuffer = nullptr;
		};

		class ComputePipeline
		{
		public:
			virtual void Cleanup(RenderingDevice* pDevice) = 0;
			Pipeline* m_pMaterialPipeline = nullptr;
		};


		class FrameCommandData
		{
		public:
			// Rendering will be split up in 2 command buffers, one for uploading data and one for using the data. 
			// If finer granularity is needed, switch to a render graph with defined passes and depednency between them. 
			enum class eCommandBuffers
			{
				Transfer,
				Graphics,
				Count
			};
			bool IsReset() { return m_currentCommandBuffer == eCommandBuffers::Count; }
			bool IsTransfer() { return m_currentCommandBuffer == eCommandBuffers::Transfer; }
			bool IsGraphics() { return m_currentCommandBuffer == eCommandBuffers::Graphics; }
			void SetTransfer() { m_currentCommandBuffer = eCommandBuffers::Transfer; }
			void SetGraphics() { m_currentCommandBuffer = eCommandBuffers::Graphics; }

			// Submits the current recorded command buffers
			virtual void Init(RenderingDevice* pDevice, uint32 frame) = 0;
			virtual void CommitCommandBuffer(RenderingDevice* pDevice) = 0;
			virtual void Reset(RenderingDevice* pDevice) = 0;
			virtual void Cleanup(RenderingDevice* pDevice, uint32 frame) = 0;

			CommandBuffer* GetCurrentCommandBuffer() { return m_pCommandBuffers[(uint32)m_currentCommandBuffer]; }
			void AddStagingBuffer(BufferObject* pBuffer) { m_stagingBuffers.Add(pBuffer); }

		protected:
			eCommandBuffers m_currentCommandBuffer = eCommandBuffers::Count;
			StaticArray<CommandBuffer*, (uint32)eCommandBuffers::Count> m_pCommandBuffers;
			GrowingArray<BufferObject*> m_stagingBuffers;

		};

		FrameCommandData* GetCurrentFrameCommandData();

		// Creates a complete state for a pipeline if it does not exist, this pipeline will be with the bound resources. 
		void ValidatePipelineAndUpdateDescriptors(Pipeline* pPipeline);
		bool CheckBoundDataAgainstSetDecoration(Pipeline* pPipeline, const SetDecoration& setDecoration, StaticArray<uint32, MaxShaderBindingCount>* pBoundListToCheck, eDecorationType decorationType);
		virtual void SubmitFinalFrameCommandBuffer() = 0;

		virtual void UploadDataToBufferInternal(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData) = 0;
		virtual void UploadDataToTextureInternal(TextureResource* pTexture, void* pDataToUpload, uint32 mipLevel) = 0;
		virtual void CopyDataToBufferInternal(BufferObject* pDstBuffer, BufferObject* pSrcBuffer) = 0;
		virtual bool BindMaterialInternal(Pipeline* pMaterial) = 0;
		virtual bool BindComputePipelineInternal(Pipeline* pPipeline) = 0;
		virtual void ClearFrameBufferInternal(FrameBufferTexture* pFrameBuffer) = 0;
		virtual void BindMaterialFrameBufferConnection(MaterialFrameBufferConnection* connectionToBind) = 0;
		virtual void BindComputePipeline(ComputePipeline* pPipelineToBind) = 0;
		virtual void BindVertexBufferInternal() = 0;
		virtual void SetPushConstantInternal(void* pPushConstant) = 0;
		virtual void TransferFramebufferLayoutInternal(TextureResource* pTextureToTransfer, eFrameBufferLayoutState sourceState, eFrameBufferLayoutState destinationState) = 0;
		virtual void TransferImageStateInternal(TextureResource* pTexture, eShaderAccessQualifier newState, uint32 newStageCombination) = 0;
		virtual void TransferBufferStateInternal(BufferObject* pBuffer, eShaderAccessQualifier newState) = 0;

		RenderingDevice* m_pDevice;
		ResourceManager* m_pResourceManager;

		eContextState m_currentState;
		uint32 m_lastBoundShaderStages;
		CommandBuffer* m_pCurrentCommandBuffer;

		Material* m_pBoundMaterial;
		Pipeline* m_pBoundMaterialPipeline;
		BufferObject* m_pBoundVertexBuffer;
		BufferObject* m_pBoundIndexBuffer;

		StaticArray<TextureView*, 16u> m_pBoundTextures;
		StaticArray<BufferObject*, 16u> m_pBoundStructuredBuffers;
		StaticArray<BufferObject*, 16u> m_pBoundUniformBuffers;
		StaticArray<FrameBufferTexture*, 8> m_pBoundFrameBuffers;

		StaticArray<FrameCommandData*, MAX_FRAMESINFLIGHT> m_pFrameCommandData;

		uint64 m_currentlyBoundPipeline{};
		eMaterialType m_boundMaterialType;

		GrowingArray<MaterialFrameBufferConnection*> m_pFrameBufferMaterialPipelines;
		GrowingArray<ComputePipeline*> m_pComputePipelines;

		uint64 m_currentRenderFrame;
	};
}