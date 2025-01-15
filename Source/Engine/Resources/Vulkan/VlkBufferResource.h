#pragma once
#include "Resources\BufferResource.h"
#include "Windows\VulkanInternal\VlkResources.h"
#include "vk_mem_alloc.h"

namespace Hail
{
	class VlkBufferObject : public BufferObject
	{
	public:
		void CleanupResource(RenderingDevice* device) override;
		bool UsesPersistentMapping(RenderingDevice* device, uint32 frameInFlight) override;
		
		const VkBuffer& GetBuffer(uint32 frameInFlight) const;
		VmaAllocation GetAllocation(uint32 frameInFlight);
		VmaAllocationInfo GetAllocationMappedMemory(uint32 frameInFlight);


	private:
		bool InternalInit(RenderingDevice* pDevice) override;

		friend class VlkTextureResourceManager;

		VmaAllocation m_allocation[MAX_FRAMESINFLIGHT];
		VmaAllocationInfo m_allocationInfo[MAX_FRAMESINFLIGHT]; // used in persistent mapping
		VkBuffer m_buffer[MAX_FRAMESINFLIGHT];
		void* m_bufferMapped[MAX_FRAMESINFLIGHT];
	};
}