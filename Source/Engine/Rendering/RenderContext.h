#pragma once
#include "Types.h"

#include "ResourceCommon.h"
#include "Resources_Materials\Materials_Common.h"

#include "Resources_Textures\TextureCommons.h"
#include "Containers\StaticArray\StaticArray.h"
#include "Containers\GrowingArray\GrowingArray.h"

namespace Hail
{
	class BufferObject;
	class FrameBufferTexture;
	class Material;
	class MaterialManager;
	class ResourceManager;
	class RenderingDevice;
	class TextureResource;
	class TextureView;
	class Pipeline;

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
		explicit CommandBuffer(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer);
		~CommandBuffer();
		void BeginBuffer();
		void EndBuffer(bool bDestroyBufferData);
	protected:
		virtual void BeginBufferInternal() = 0;
		virtual void EndBufferInternal(bool bDestroyBufferData) = 0;
		friend class RenderContext;
		const eContextState m_contextState;
		const RenderingDevice* m_pDevice;
		bool m_bIsRecording;
	};

	class RenderContext
	{
	public:
		RenderContext(RenderingDevice* device, ResourceManager* pResourceManager);
		virtual void Cleanup() = 0;

		void SetBufferAtSlot(BufferObject* pBuffer, uint32 slot);
		void SetTextureAtSlot(TextureView* pTexture, uint32 slot);

		void BindMaterial(Material* pMaterial);
		void BindMaterial(Pipeline* pPipeline);
		// Will bind the instance ID for the currently bound material.
		virtual void BindMaterialInstance(uint32 materialInstanceIndex) = 0;
		void BindFrameBufferAtSlot(FrameBufferTexture* pFrameBuffer, uint32 bindSlot);
		void BindVertexBuffer(BufferObject* pVertexBufferToBind, BufferObject* pIndexBufferToBind);
		// Creates a complete state for a pipeline if it does not exist, this pipeline will be with the bound resources. 
		void SetPipelineState(Pipeline* pPipeline);
		// Takes any 16 byte sized data
		void SetPushConstantValue(void* pPushConstant);

		// Clears the framebuffers colors to the base colors and depth values
		void ClearBoundFrameBuffers();

		// To clean up internal states and set them to invalid for validation and cleanup.
		virtual void DeleteFramebuffer(FrameBufferTexture* pFrameBufferToDelete);
		virtual void DeleteMaterial(Material* pMaterialToDelete);

		TextureView* GetBoundTextureAtSlot(uint32 slot);
		BufferObject* GetBoundStructuredBufferAtSlot(uint32 slot);
		BufferObject* GetBoundUniformBufferAtSlot(uint32 slot);

		void UploadDataToBuffer(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData);
		void UploadDataToTexture(TextureResource* pTexture, void* pDataToUpload, uint32 mipLevel);

		void TransferFramebufferLayout(FrameBufferTexture* pTextureToTransfer, eFrameBufferLayoutState colorState, eFrameBufferLayoutState depthState);

		virtual void RenderMeshlets(glm::uvec3 dispatchSize);
		virtual void RenderSprites(uint32 numberOfInstances, uint32 offset) = 0;

		CommandBuffer* GetCurrentCommandBuffer() { return m_pCurrentCommandBuffer; }
		// Will end the frame if the last bound material that was bound is FULLSCREEN_PRESENT_LETTERBOX
		void StartGraphicsPass();
		void EndGraphicsPass();
		//void StartComputePass();
		//void EndComputePass();
		void StartTransferPass();
		void EndTransferPass();

		virtual void StartFrame() = 0;
		virtual void EndRenderPass() = 0;

	protected:

		class MaterialFrameBufferConnection
		{
		public:
			virtual void Cleanup(RenderingDevice* pDevice) = 0;
			ResourceValidator m_validator;
			Pipeline* m_pMaterialPipeline = nullptr;
			FrameBufferTexture* m_pBoundFrameBuffer = nullptr;
		};

		void Init();

		virtual void SubmitFinalFrameCommandBuffer() = 0;

		virtual CommandBuffer* CreateCommandBufferInternal(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer) = 0;
		virtual void UploadDataToBufferInternal(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData) = 0;
		virtual void UploadDataToTextureInternal(TextureResource* pTexture, void* pDataToUpload, uint32 mipLevel) = 0;
		virtual void TransferFramebufferLayoutInternal(TextureResource* pTextureToTransfer, eFrameBufferLayoutState sourceState, eFrameBufferLayoutState destinationState) = 0;
		virtual bool BindMaterialInternal(Pipeline* pMaterial) = 0;
		virtual void ClearFrameBufferInternal(FrameBufferTexture* pFrameBuffer) = 0;
		virtual MaterialFrameBufferConnection* CreateMaterialFrameBufferConnection() = 0;
		virtual void BindMaterialFrameBufferConnection(MaterialFrameBufferConnection* connectionToBind) = 0;
		virtual void BindVertexBufferInternal() = 0;
		virtual void SetPushConstantInternal(void* pPushConstant) = 0;


		RenderingDevice* m_pDevice;
		ResourceManager* m_pResourceManager;

		eContextState m_currentState;
		CommandBuffer* m_pCurrentCommandBuffer;

		Material* m_pBoundMaterial;
		Pipeline* m_pBoundMaterialPipeline;
		BufferObject* m_pBoundVertexBuffer;
		BufferObject* m_pBoundIndexBuffer;

		StaticArray<TextureView*, 16> m_pBoundTextures;
		StaticArray<BufferObject*, 16> m_pBoundStructuredBuffers;
		StaticArray<BufferObject*, 16> m_pBoundUniformBuffers;
		StaticArray<FrameBufferTexture*, 8> m_pBoundFrameBuffers;

		GrowingArray<BufferObject*> m_stagingBuffers;

		StaticArray<CommandBuffer*, MAX_FRAMESINFLIGHT> m_pGraphicsCommandBuffers;

		uint64 m_currentlyBoundPipeline{};
		eMaterialType m_boundMaterialType;

		GrowingArray<MaterialFrameBufferConnection*> m_pFrameBufferMaterialPipelines;
	};
}