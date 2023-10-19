#include "Engine_PCH.h"
#include "VlkTextureManager.h"
#include "Windows\VulkanInternal\VlkDevice.h"
#include "Windows\VulkanInternal\VlkBufferCreationFunctions.h"
#include "Utility\StringUtility.h"

using namespace Hail;

void VlkTextureResourceManager::Init(RenderingDevice* device)
{
	m_device = reinterpret_cast<VlkDevice*>(device);
	m_textures.Init(10);
	TextureManager::Init(device);
}

void VlkTextureResourceManager::ClearAllResources()
{
	for (size_t i = 0; i < m_textures.Size(); i++)
	{
		m_textures[i].CleanupResource(m_device);
	}
	m_textures.RemoveAll();
}

bool VlkTextureResourceManager::LoadTexture(const char* textureName)
{
	TextureResource textureResource;
	VlkTextureResource vlkTextureResource;
	if (LoadTextureInternal(textureName, textureResource, false))
	{
		textureResource.index = m_textureCommonData.Size();
		m_textureCommonData.Add(textureResource);
		if (!CreateTextureData(m_textureCommonData.GetLast().m_compiledTextureData, vlkTextureResource.GetVlkTextureData()))
		{
			vlkTextureResource.CleanupResource(m_device);
			DeleteCompiledTexture(textureResource.m_compiledTextureData);
			return false;
		}
	}
	m_textures.Add(vlkTextureResource);

	return true;
}


void Hail::VlkTextureResourceManager::ClearTextureInternalForReload(int textureIndex, uint32 frameInFlight)
{
	TextureManager::ClearTextureInternalForReload(textureIndex, frameInFlight);
	m_textures[textureIndex].CleanupResourceForReload(m_device, frameInFlight);
}

bool Hail::VlkTextureResourceManager::ReloadTextureInternal(int textureIndex, uint32 frameInFlight)
{
	if (!TextureManager::ReloadTextureInternal(textureIndex, frameInFlight))
	{
		return false;
	}
 	if (m_textures[textureIndex].GetValidator().IsAllFrameResourcesDirty() && !CreateTextureData(m_textureCommonData[textureIndex].m_compiledTextureData, m_textures[textureIndex].GetVlkTextureData()))
	{
		m_textures[textureIndex].CleanupResource(m_device);
		DeleteCompiledTexture(m_textureCommonData[textureIndex].m_compiledTextureData);
		return false;
	}
	m_textures[textureIndex].GetValidator().ClearFrameData(frameInFlight);
	return true;
}

bool VlkTextureResourceManager::CreateTextureData(CompiledTexture& compiledTexture, VlkTextureData& vlkTextureData)
{
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

	VkFormat textureFormat = ToVkFormat(TextureTypeToTextureFormat(static_cast<TEXTURE_TYPE>(compiledTexture.header.textureType)));

	CreateImage(device, compiledTexture.header.width, compiledTexture.header.height,
		textureFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vlkTextureData.textureImage, vlkTextureData.textureImageMemory);

	TransitionImageLayout(device, vlkTextureData.textureImage, textureFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device.GetCommandPool(), device.GetGraphicsQueue());
	CopyBufferToImage(device, stagingBuffer, vlkTextureData.textureImage, compiledTexture.header.width, compiledTexture.header.height, device.GetCommandPool(), device.GetGraphicsQueue());
	TransitionImageLayout(device, vlkTextureData.textureImage, textureFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device.GetCommandPool(), device.GetGraphicsQueue());

	vkDestroyBuffer(device.GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), stagingBufferMemory, nullptr);

	vlkTextureData.textureImageView = CreateImageView(device, vlkTextureData.textureImage, textureFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	return true;
}


VlkTextureData& VlkTextureResourceManager::GetTextureData(uint32_t index)
{
	return m_textures[index].GetVlkTextureData();
}

FrameBufferTexture* VlkTextureResourceManager::FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat)
{
	VlkFrameBufferTexture* frameBuffer = new VlkFrameBufferTexture(resolution, format, depthFormat);
	frameBuffer->SetName(name);
	frameBuffer->CreateFrameBufferTextureObjects(m_device);
	return frameBuffer;
}