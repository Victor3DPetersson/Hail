#pragma once
#include "Types.h"

#include "Containers\StaticArray\StaticArray.h"
#include "Containers\GrowingArray\GrowingArray.h"

namespace Hail
{
	class BufferObject;
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
		explicit CommandBuffer(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer, bool bIsTempCommandBuffer);
		~CommandBuffer();
		virtual void EndBuffer() = 0;
	protected:
		friend class RenderContext;
		const eContextState m_contextState;
		const bool m_bIsTempCommandBuffer;
		const RenderingDevice* m_pDevice;
		bool m_bIsInitialized;
	};

	class RenderContext
	{
	public:
		RenderContext(RenderingDevice* device, ResourceManager* pResourceManager);

		void SetBufferAtSlot(BufferObject* pBuffer, uint32 slot);
		void SetTextureAtSlot(TextureView* pTexture, uint32 slot);

		// Creates a complete state for a pipeline if it does not exist, this pipeline will be with the bound resources. 
		void SetPipelineState(Pipeline* pPipeline);

		TextureView* GetBoundTextureAtSlot(uint32 slot);
		BufferObject* GetBoundStructuredBufferAtSlot(uint32 slot);
		BufferObject* GetBoundUniformBufferAtSlot(uint32 slot);

		void UploadDataToBuffer(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData);
		void UploadDataToTexture(TextureResource* pTexture, void* pDataToUpload, uint32 mipLevel);

		CommandBuffer* GetCurrentCommandBuffer() { return m_pCurrentCommandBuffer; }
		//void StartGraphicsPass();
		//void EndGraphicsPass();
		//void StartComputePass();
		//void EndComputePass();
		void StartTransferPass();
		void EndTransferPass();

	protected:

		virtual CommandBuffer* CreateCommandBufferInternal(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer, bool bIsTempCommandBuffer) = 0;
		virtual void UploadDataToBufferInternal(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData) = 0;
		virtual void UploadDataToTextureInternal(TextureResource* pTexture, void* pDataToUpload, uint32 mipLevel) = 0;
		StaticArray<TextureView*, 16> m_pBoundTextures;
		StaticArray<BufferObject*, 16> m_pBoundStructuredBuffers;
		StaticArray<BufferObject*, 16> m_pBoundUniformBuffers;

		eContextState m_currentState;
		CommandBuffer* m_pCurrentCommandBuffer;

		RenderingDevice* m_pDevice;
		ResourceManager* m_pResourceManager;

		GrowingArray<BufferObject*> m_stagingBuffers;
	};
}