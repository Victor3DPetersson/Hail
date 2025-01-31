#pragma once
#include "..\TextureResource.h"
#include "Windows\VulkanInternal\VlkResources.h"
#include "vk_mem_alloc.h"

namespace Hail
{
	class RenderingDevice;
	class VlkTextureResource : public TextureResource
	{
	public:
		struct VlkTextureInternalData
		{
			VmaAllocation allocation = VK_NULL_HANDLE;
			VkImage textureImage = VK_NULL_HANDLE;

			// TODO: Add current texture state so we can track it for transitions
		};

		void CleanupResource(RenderingDevice* device) override;
		void CleanupResourceForReload(RenderingDevice* device, uint32 frameInFligth) override;
		VlkTextureInternalData& GetVlkTextureData() { return m_textureData; }
		ResourceValidator& GetValidator() { return m_validator; }
	private:
		friend class VlkTextureResourceManager;
		bool InternalInit(RenderingDevice* pDevice) override;
		//VlkTextureData m_textureData;
		//VlkTextureData m_unloadingTextureData;

		VlkTextureInternalData m_textureData;
		VlkTextureInternalData m_unloadingTextureData;

		ResourceValidator m_validator = ResourceValidator();
	};

	class VlkTextureView : public TextureView
	{
	public:

		virtual void CleanupResource(RenderingDevice* pDevice);
		virtual bool InitView(RenderingDevice* pDevice, TextureViewProperties properties);

		VkImageView& GetVkImageView() { return m_textureImageView; }

	private:

		VkImageView m_textureImageView = VK_NULL_HANDLE;
	};

	class ImGuiVlkTextureResource : public ImGuiTextureResource
	{
	public:
		void* GetImguiTextureResource() final { return &m_ImGuiResource; }

	private:
		friend class VlkTextureResourceManager;

		VkDescriptorSet m_ImGuiResource = VK_NULL_HANDLE;
		VkImageView     m_imageView = VK_NULL_HANDLE;
		VmaAllocation	m_allocation = VK_NULL_HANDLE;
		VkImage         m_image = VK_NULL_HANDLE;
		//VkBuffer        m_uploadBuffer;
		//VkDeviceMemory  m_uploadBufferMemory;
	};
}