#include "Engine_PCH.h"
#include "VlkTextureManager.h"
#include "Windows\VulkanInternal\VlkDevice.h"
#include "Windows\VulkanInternal\VlkBufferCreationFunctions.h"
#include "Utility\StringUtility.h"
#include "Windows\VulkanInternal\VlkResourceManager.h"

#include "Utility\FilePath.hpp"
#include "Utility\InOutStream.h"


#include "Windows/imgui_impl_vulkan.h"
#include "MetaResource.h"


using namespace Hail;

void VlkTextureResourceManager::Init(RenderingDevice* device)
{
	m_device = reinterpret_cast<VlkDevice*>(device);
	TextureManager::Init(device);
}

TextureResource* VlkTextureResourceManager::CreateTextureInternal(const char* name, CompiledTexture& compiledTextureData)
{
	VlkTextureResource* vlkTextureResource = new VlkTextureResource();
	vlkTextureResource->textureName = name;
	if (!CreateTextureData(compiledTextureData, vlkTextureResource->GetVlkTextureData()))
	{
		vlkTextureResource->CleanupResource(m_device);
		DeleteCompiledTexture(compiledTextureData);
		return false;
	}
	vlkTextureResource->m_compiledTextureData = compiledTextureData;
	return vlkTextureResource;
}

void Hail::VlkTextureResourceManager::ClearTextureInternalForReload(int textureIndex, uint32 frameInFlight)
{
	TextureManager::ClearTextureInternalForReload(textureIndex, frameInFlight);
	m_loadedTextures[textureIndex]->CleanupResourceForReload(m_device, frameInFlight);
}

bool Hail::VlkTextureResourceManager::ReloadTextureInternal(int textureIndex, uint32 frameInFlight)
{
	if (!TextureManager::ReloadTextureInternal(textureIndex, frameInFlight))
	{
		return false;
	}
 	if (m_loadedTextures[textureIndex]->m_validator.IsAllFrameResourcesDirty() && !CreateTextureData(m_loadedTextures[textureIndex]->m_compiledTextureData, ((VlkTextureResource*)m_loadedTextures[textureIndex])->GetVlkTextureData()))
	{
		m_loadedTextures[textureIndex]->CleanupResource(m_device);
		DeleteCompiledTexture(m_loadedTextures[textureIndex]->m_compiledTextureData);
		return false;
	}
	m_loadedTextures[textureIndex]->m_validator.ClearFrameData(frameInFlight);
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

FrameBufferTexture* VlkTextureResourceManager::FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat)
{
	VlkFrameBufferTexture* frameBuffer = new VlkFrameBufferTexture(resolution, format, depthFormat);
	frameBuffer->SetName(name);
	frameBuffer->CreateFrameBufferTextureObjects(m_device);
	return frameBuffer;
}

#pragma optimize("", off)
ImGuiTextureResource* Hail::VlkTextureResourceManager::CreateImGuiTextureResource(const FilePath& filepath, RenderingResourceManager* renderingResourceManager, TextureHeader* headerToFill)
{
	const FileObject& object = filepath.Object();
	MetaResource metaData;
	CompiledTexture compiledTextureData;

	if (StringCompare(object.Extension(), L"txr"))
	{
		InOutStream inStream;
		if (!inStream.OpenFile(filepath, FILE_OPEN_TYPE::READ, true))
		{
			H_ASSERT(false, "Creating ImGui image with a path from a tool should not be possible.");
			return nullptr;
		}

		if (!ReadStreamInternal(compiledTextureData, inStream, metaData))
		{
			H_ERROR(String256::Format("Failed to import ImGui Texture: %", filepath.Object().Name()));
			return nullptr;
		}

		inStream.CloseFile();
		compiledTextureData.loadState = TEXTURE_LOADSTATE::LOADED_TO_RAM;
	}
	else
	{
		//TODO: import external texture 
	}

	ImGuiVlkTextureResource* returnResource = new ImGuiVlkTextureResource();
	returnResource->metaDataOfResource = metaData;
	VlkDevice& device = *m_device;

	if (headerToFill)
		*headerToFill = compiledTextureData.header;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	const uint32_t imageSize = GetTextureByteSize(compiledTextureData.header);

	CreateBuffer(device, imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device.GetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, compiledTextureData.compiledColorValues, static_cast<size_t>(imageSize));
	vkUnmapMemory(device.GetDevice(), stagingBufferMemory);
	DeleteCompiledTexture(compiledTextureData);

	VkFormat textureFormat = ToVkFormat(TextureTypeToTextureFormat((TEXTURE_TYPE)compiledTextureData.header.textureType));

	CreateImage(device, compiledTextureData.header.width, compiledTextureData.header.height,
		textureFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		returnResource->m_image, returnResource->m_uploadBufferMemory);

	TransitionImageLayout(device, returnResource->m_image, textureFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device.GetCommandPool(), device.GetGraphicsQueue());
	CopyBufferToImage(device, stagingBuffer, returnResource->m_image, compiledTextureData.header.width, compiledTextureData.header.height, device.GetCommandPool(), device.GetGraphicsQueue());
	TransitionImageLayout(device, returnResource->m_image, textureFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device.GetCommandPool(), device.GetGraphicsQueue());

	vkDestroyBuffer(device.GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), stagingBufferMemory, nullptr);

	returnResource->m_imageView = CreateImageView(device, returnResource->m_image, textureFormat, VK_IMAGE_ASPECT_COLOR_BIT);

	returnResource->m_height = compiledTextureData.header.height;
	returnResource->m_width = compiledTextureData.header.width;

	VlkRenderingResources* vlkRenderingResources = (VlkRenderingResources*)renderingResourceManager->GetRenderingResources();
	// Create Descriptor Set using ImGUI's implementation
	returnResource->m_ImGuiResource = ImGui_ImplVulkan_AddTexture(vlkRenderingResources->m_linearTextureSampler, returnResource->m_imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	return returnResource;
}
#pragma optimize("", on)
void Hail::VlkTextureResourceManager::DeleteImGuiTextureResource(ImGuiTextureResource* textureToDelete)
{
	VlkDevice& device = *m_device;
	ImGuiVlkTextureResource* textureData = (ImGuiVlkTextureResource*)textureToDelete;
	if (textureData->m_imageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(device.GetDevice(), textureData->m_imageView, nullptr);
	}
	if (textureData->m_image != VK_NULL_HANDLE)
	{
		vkDestroyImage(device.GetDevice(), textureData->m_image, nullptr);
	}
	if (textureData->m_uploadBufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device.GetDevice(), textureData->m_uploadBufferMemory, nullptr);
	}
	textureData->m_imageView = VK_NULL_HANDLE;
	textureData->m_image = VK_NULL_HANDLE;
	textureData->m_uploadBufferMemory = VK_NULL_HANDLE;
}
