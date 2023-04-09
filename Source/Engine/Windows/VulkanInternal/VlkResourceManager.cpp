#include "Engine_PCH.h"
#include "VlkResourceManager.h"
#include "Resources\TextureCommons.h"

#include "VlkDevice.h"
#include "VlkBufferCreationFunctions.h"
#include "VlkTextureCreationFunctions.h"

namespace Hail
{


    void VlkResourceManager::Init(RenderingDevice* device)
    {
        m_device = reinterpret_cast<VlkDevice*>(device);
		m_textureData.Init(10);
    }
    void VlkResourceManager::CreateMaterialDescriptor()
    {
    }

	bool VlkResourceManager::CreateTextureData(CompiledTexture& compiledTexture)
    {
		VlkTextureData vlkTextureData{};

		VlkDevice& device = *m_device;
		if (compiledTexture.loadState != TEXTURE_LOADSTATE::LOADED_TO_RAM)
		{
			return false;
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		const uint32_t imageSize = GetTextureByteSize(compiledTexture.header);

		CreateBuffer(device, imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device.GetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, compiledTexture.compiledColorValues, static_cast<size_t>(imageSize));
		vkUnmapMemory(device.GetDevice(), stagingBufferMemory);
		DeleteCompiledTexture(compiledTexture);

		//Todo, add support for more formats
		CreateImage(device, compiledTexture.header.width, compiledTexture.header.height,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vlkTextureData.textureImage, vlkTextureData.textureImageMemory);

		TransitionImageLayout(device, vlkTextureData.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device.GetCommandPool(), device.GetGraphicsQueue());
		CopyBufferToImage(device, stagingBuffer, vlkTextureData.textureImage, compiledTexture.header.width, compiledTexture.header.height, device.GetCommandPool(), device.GetGraphicsQueue());
		TransitionImageLayout(device, vlkTextureData.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device.GetCommandPool(), device.GetGraphicsQueue());

		vkDestroyBuffer(device.GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device.GetDevice(), stagingBufferMemory, nullptr);

		vlkTextureData.textureImageView = CreateImageView(device, vlkTextureData.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

		m_textureData.Add(vlkTextureData);

		return true;
    }

    VlkMaterialDescriptor& VlkResourceManager::GetMaterialData(uint32_t index)
    {
		return m_materialDescriptors[index];
    }
    VlkTextureData& VlkResourceManager::GetTextureData(uint32_t index)
    {
		return m_textureData[index];
    }
}