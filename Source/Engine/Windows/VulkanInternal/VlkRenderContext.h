#pragma once

#include "Rendering\RenderContext.h"
#include "VlkDevice.h"

namespace Hail
{
	class VlkBufferObject;

	class VlkCommandBuffer : public CommandBuffer
	{
	public:
		VlkCommandBuffer(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer, bool bIsTempCommandBuffer);
		void EndBuffer() override;
		
		friend class VlkRenderContext;
		friend class VlkRenderer;
	private:
		VkCommandBuffer m_commandBuffer;
	};

	class VlkRenderContext : public RenderContext
	{
	public:
		explicit VlkRenderContext(RenderingDevice* device, ResourceManager* pResourceManager);

		CommandBuffer* CreateCommandBufferInternal(RenderingDevice* pDevice, eContextState contextStateForCommandBuffer, bool bIsTempCommandBuffer) override;
		void UploadDataToBufferInternal(BufferObject* pBuffer, void* pDataToUpload, uint32 sizeOfUploadedData) override;
		void UploadDataToTextureInternal(TextureResource* pTexture, void* pDataToUpload, uint32 mipLevel);

	private:

		VlkBufferObject* CreateStagingBufferAndMemoryBarrier(uint32 bufferSize, void* pDataToUpload);
	};

}

