#pragma once
#include "Resources\BufferResource.h"
#include "Windows\VulkanInternal\VlkResources.h"

namespace Hail
{
	class VlkBufferObject : public BufferObject
	{
	public:
		bool Init(RenderingDevice* device, BufferProperties properties) override;
		void CleanupResource(RenderingDevice* device) override;
		const VkBuffer& GetBuffer(uint32 frameInFlight) const { return m_buffer[frameInFlight]; }
		void* GetMappedMemory(uint32 frameInFlight) { return m_bufferMapped[frameInFlight]; }

	private:
		friend class VlkTextureResourceManager;

		VkBuffer m_buffer[MAX_FRAMESINFLIGHT];
		VkDeviceMemory m_bufferMemory[MAX_FRAMESINFLIGHT];
		void* m_bufferMapped[MAX_FRAMESINFLIGHT];
	};
}