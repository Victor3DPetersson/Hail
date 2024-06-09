#pragma once
#include "..\TextureResource.h"
#include "Windows\VulkanInternal\VlkResources.h"

namespace Hail
{
	class RenderingDevice;
	class VlkTextureResource : public TextureResource
	{
	public:

		void CleanupResource(RenderingDevice* device) override;
		void CleanupResourceForReload(RenderingDevice* device, uint32 frameInFligth) override;
		VlkTextureData& GetVlkTextureData() { return m_textureData; }
		ResourceValidator& GetValidator() { return m_validator; }
	private:
		VlkTextureData m_textureData;
		VlkTextureData m_unloadingTextureData;
		ResourceValidator m_validator = ResourceValidator();
	};



	class ImGuiVlkTextureResource : public ImGuiTextureResource
	{
	public:

		void* GetImguiTextureResource() final { return &m_ImGuiResource; }

	private:
		friend class VlkTextureResourceManager;

		VkDescriptorSet m_ImGuiResource;
		VkImageView     m_imageView;
		VkImage         m_image;
		VkBuffer        m_uploadBuffer;
		VkDeviceMemory  m_uploadBufferMemory;
	};
}