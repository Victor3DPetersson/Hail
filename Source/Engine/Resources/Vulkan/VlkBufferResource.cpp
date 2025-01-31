#include "Engine_PCH.h"

#include "VlkTextureResource.h"
#include "Windows\VulkanInternal\VlkDevice.h"
#include "VlkBufferResource.h"

using namespace Hail;

uint32 g_numberOfRegisteredBuffers = 0u;
namespace
{
	VkResult CreateBufferInternal(VlkDevice& device, VkMemoryPropertyFlags bufferType, VkBuffer& buffer, VmaAllocation& allocation, BufferProperties& bufferProps, VmaAllocationInfo* pAllocationInfo)
	{
		VkDeviceSize size = bufferProps.elementByteSize * bufferProps.numberOfElements;
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.usage = bufferType;

		VmaAllocationCreateInfo vmaInfo{};
		vmaInfo.usage = VMA_MEMORY_USAGE_AUTO;
		vmaInfo.pool = VK_NULL_HANDLE;

		VkBufferUsageFlags usage{};
		if (bufferProps.domain == eShaderBufferDomain::GpuOnly)
		{
			vmaInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			usage |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			if (bufferProps.updateFrequency == eShaderBufferUpdateFrequency::Once)
				usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		else if (bufferProps.domain == eShaderBufferDomain::GpuToCpu)
		{
			vmaInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | 
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | 
				VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
			if (bufferProps.updateFrequency == eShaderBufferUpdateFrequency::PerFrame)
				vmaInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

			usage |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}
		else if (bufferProps.domain == eShaderBufferDomain::CpuToGpu)
		{
			if (bufferProps.type == eBufferType::staging)
			{
				vmaInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
					VMA_ALLOCATION_CREATE_MAPPED_BIT;
			}
			else
			{
				vmaInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
					VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
				if (bufferProps.updateFrequency == eShaderBufferUpdateFrequency::PerFrame)
					vmaInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

				usage |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			}
		}

		bufferInfo.usage |= usage;
		// for random access  
		//flags: VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
		// required_flags : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT.

		/*flags breakdown

		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		image attachments,
		compute buffers,
		3D object textures and buffers,
		everything that rarely changes and favors fast access.


		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT. Make memory visible from the host.
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT. Alleviates need to call vkFlushMappedMemoryRanges() (or vkInvalidateMappedMemoryRanges()) after each CPU (or GPU) write to the mapped memory. Assures availability. Does not guarantee visibility.
		VK_MEMORY_PROPERTY_HOST_CACHED_BIT


		MemoryLocation::GpuOnly - VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT.
		MemoryLocation::CpuToGpu - VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT. Coherent, host-visible memory is ideal for mapped memory. All modern GPUs have about 256MB accessible from the CPU with little performance penalty. This is known as the Base Address Register (BAR).
		MemoryLocation::GpuToCpu - VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT

		VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT. Often used, GPU-allocated resources. Image attachments, compute buffers, static 3D object textures and buffers, etc.
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT. Temporary staging buffer that the CPU will write to. Its content will be then copied to a high-performance GPU memory.
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT. Mapped memory when you want to read the GPU’s memory from the CPU.

		for mapping memory VMA_ALLOCATION_CREATE_MAPPED_BIT

		*/
		g_numberOfRegisteredBuffers++;
		return vmaCreateBuffer(device.GetMemoryAllocator(), &bufferInfo, &vmaInfo, &buffer, &allocation, pAllocationInfo);
	}

}

void Hail::VlkBufferObject::CleanupResource(RenderingDevice* device)
{
	VlkDevice& vkDevice = *(VlkDevice*)device;

	if (m_bUsesFramesInFlight)
	{
		for (uint32 i = 0; i < MAX_FRAMESINFLIGHT; i++)
		{
			if (m_buffer[i] != VK_NULL_HANDLE && m_allocation[i])
			{
				vmaDestroyBuffer(vkDevice.GetMemoryAllocator(), m_buffer[i], m_allocation[i]);
			}
			m_buffer[i] = VK_NULL_HANDLE;
			m_allocation[i] = VK_NULL_HANDLE;
			g_numberOfRegisteredBuffers--;
		}

	}
	else
	{
		if (m_buffer[0] != VK_NULL_HANDLE && m_allocation[0])
		{
			vmaDestroyBuffer(vkDevice.GetMemoryAllocator(), m_buffer[0], m_allocation[0]);
		}
		m_buffer[0] = VK_NULL_HANDLE;
		m_allocation[0] = VK_NULL_HANDLE;
		g_numberOfRegisteredBuffers--;
	}
	m_properties = {};
}

bool Hail::VlkBufferObject::UsesPersistentMapping(RenderingDevice* device, uint32 frameInFlight)
{
	VlkDevice& vkDevice = *(VlkDevice*)device;

	const uint32 allocatorIndex = m_bUsesFramesInFlight ? frameInFlight : 0u;

	VkMemoryPropertyFlags memPropFlags;
	vmaGetAllocationMemoryProperties(vkDevice.GetMemoryAllocator(), m_allocation[allocatorIndex], &memPropFlags);

	if (memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		return true;

	return false;
}

const VkBuffer& Hail::VlkBufferObject::GetBuffer(uint32 frameInFlight) const
{
	if (m_bUsesFramesInFlight)
		return m_buffer[frameInFlight];
	else
		return m_buffer[0];
}

VmaAllocation Hail::VlkBufferObject::GetAllocation(uint32 frameInFlight)
{
	H_ASSERT(m_allocation);
	if (m_bUsesFramesInFlight)
		return m_allocation[frameInFlight];
	else
		return m_allocation[0];
}

VmaAllocationInfo Hail::VlkBufferObject::GetAllocationMappedMemory(uint32 frameInFlight)
{
	H_ASSERT(m_allocationInfo);
	if (m_bUsesFramesInFlight)
		return m_allocationInfo[frameInFlight];
	else
		return VmaAllocationInfo();
}

bool Hail::VlkBufferObject::InternalInit(RenderingDevice* pDevice)
{
	VlkDevice& vlkDevice = *(VlkDevice*)pDevice;

	VkBufferUsageFlags typeFlag;
	if (m_properties.type == eBufferType::uniform)
	{
		typeFlag = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}
	else if (m_properties.type == eBufferType::structured)
	{
		typeFlag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}
	else if (m_properties.type == eBufferType::index)
	{
		typeFlag = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}
	else if (m_properties.type == eBufferType::vertex)
	{
		typeFlag = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}
	else if (m_properties.type == eBufferType::staging)
	{
		typeFlag = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	}
	else
	{
		H_ASSERT(false);
		return false;
	}

	if (m_properties.updateFrequency == eShaderBufferUpdateFrequency::Once || m_properties.updateFrequency == eShaderBufferUpdateFrequency::Never)
	{
		if (CreateBufferInternal(vlkDevice, typeFlag, m_buffer[0], m_allocation[0], m_properties, nullptr) != VK_SUCCESS)
		{
			return false;
		}
	}
	else
	{
		for (uint32 i = 0; i < MAX_FRAMESINFLIGHT; i++)
		{
			if (CreateBufferInternal(vlkDevice, typeFlag, m_buffer[i], m_allocation[i], m_properties, &m_allocationInfo[i]) != VK_SUCCESS)
			{
				return false;
			}
		}
	}
	return true;
}
