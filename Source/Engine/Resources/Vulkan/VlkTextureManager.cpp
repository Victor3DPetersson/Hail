#include "Engine_PCH.h"
#include "VlkTextureManager.h"
#include "Windows\VulkanInternal\VlkDevice.h"
#include "Windows\VulkanInternal\VlkResourceManager.h"
#include "Rendering\RenderContext.h"

#include "Utility\FilePath.hpp"
#include "Utility\InOutStream.h"
#include "Utility\StringUtility.h"

#include "Windows/imgui_impl_vulkan.h"
#include "MetaResource.h"


using namespace Hail;

TextureResource* VlkTextureResourceManager::CreateTextureInternal(const char* name, CompiledTexture& compiledTextureData)
{
	VlkTextureResource* vlkTextureResource = new VlkTextureResource();
	vlkTextureResource->textureName = name;
	//if (!CreateTextureData(compiledTextureData, vlkTextureResource->GetVlkTextureData()))
	{
		//vlkTextureResource->CleanupResource(m_device);
		//DeleteCompiledTexture(compiledTextureData);
		//return false;
	}
	vlkTextureResource->m_compiledTextureData = compiledTextureData;
	return vlkTextureResource;
}

TextureResource* Hail::VlkTextureResourceManager::CreateTextureInternalNoLoad()
{
	return new VlkTextureResource();
}

bool Hail::VlkTextureResourceManager::CreateTextureGPUData(RenderContext* pRenderContext, CompiledTexture& compiledTextureData, TextureResource* pTextureResource)
{
	VlkDevice& device = *(VlkDevice*)m_device;
	if (compiledTextureData.loadState != TEXTURE_LOADSTATE::LOADED_TO_RAM)
	{
		return false;
	}

	if (!pTextureResource->Init(m_device))
		return false;

	return true;
}

void Hail::VlkTextureResourceManager::ClearTextureInternalForReload(int textureIndex, uint32 frameInFlight)
{
	TextureManager::ClearTextureInternalForReload(textureIndex, frameInFlight);
	m_loadedTextures[textureIndex].m_pTexture->CleanupResourceForReload(m_device, frameInFlight);
	m_loadedTextures[textureIndex].m_pView->CleanupResource(m_device);
}

bool Hail::VlkTextureResourceManager::ReloadTextureInternal(int textureIndex, uint32 frameInFlight)
{
	if (!TextureManager::ReloadTextureInternal(textureIndex, frameInFlight))
	{
		return false;
	}
	H_ASSERT(false);
	// TODO: Fixa omladdning
 	//if (m_loadedTextures[textureIndex]->m_validator.IsAllFrameResourcesDirty() && !CreateTextureData(m_loadedTextures[textureIndex]->m_compiledTextureData, ((VlkTextureResource*)m_loadedTextures[textureIndex])->GetVlkTextureData()))
	{
		m_loadedTextures[textureIndex].m_pTexture->CleanupResource(m_device);
		DeleteCompiledTexture(m_loadedTextures[textureIndex].m_pTexture->m_compiledTextureData);
		return false;
	}
	m_loadedTextures[textureIndex].m_pTexture->m_validator.ClearFrameData(frameInFlight);
	return true;
}

bool VlkTextureResourceManager::CreateTextureData(CompiledTexture& compiledTexture, VlkTextureData& vlkTextureData)
{
	VlkDevice& device = *(VlkDevice*)m_device;
	if (compiledTexture.loadState != TEXTURE_LOADSTATE::LOADED_TO_RAM)
	{
		return false;
	}
	H_ASSERT(false, "TODO: Remove this function, do not get here");
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	const uint32_t imageSize = GetTextureByteSize(compiledTexture.properties);

	//CreateBuffer(device, imageSize,
	//	VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	//	stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device.GetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, compiledTexture.compiledColorValues, static_cast<size_t>(imageSize));
	vkUnmapMemory(device.GetDevice(), stagingBufferMemory);
	DeleteCompiledTexture(compiledTexture);

	VkFormat textureFormat = ToVkFormat(compiledTexture.properties.format);

	//CreateImage(device, compiledTexture.properties.width, compiledTexture.properties.height,
	//	textureFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	//	VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	//	vlkTextureData.textureImage, vlkTextureData.textureImageMemory);
	// TODO record all these commands in one command, also move this to the rendering context
	//TransitionImageLayout(device, vlkTextureData.textureImage, textureFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device.GetCommandPool(), device.GetGraphicsQueue());
	//CopyBufferToImage(device, stagingBuffer, vlkTextureData.textureImage, compiledTexture.properties.width, compiledTexture.properties.height, device.GetCommandPool(), device.GetGraphicsQueue());
	//TransitionImageLayout(device, vlkTextureData.textureImage, textureFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device.GetCommandPool(), device.GetGraphicsQueue());

	vkDestroyBuffer(device.GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetDevice(), stagingBufferMemory, nullptr);

	vlkTextureData.textureImageView = nullptr;// CreateImageView(device, vlkTextureData.textureImage, textureFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	return true;
}

Hail::VlkTextureResourceManager::VlkTextureResourceManager(RenderingDevice* pDevice) : TextureManager(pDevice)
{
}

FrameBufferTexture* VlkTextureResourceManager::FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, eTextureFormat format, TEXTURE_DEPTH_FORMAT depthFormat)
{
	VlkFrameBufferTexture* frameBuffer = new VlkFrameBufferTexture(resolution, format, depthFormat);
	frameBuffer->SetName(name);
	frameBuffer->CreateFrameBufferTextureObjects(m_device);
	return frameBuffer;
}

TextureView* Hail::VlkTextureResourceManager::CreateTextureView()
{
	return new VlkTextureView();
}

#pragma optimize("", off)
ImGuiTextureResource* Hail::VlkTextureResourceManager::CreateImGuiTextureResource(RenderContext* pRenderContext, const FilePath& filepath, RenderingResourceManager* renderingResourceManager, TextureProperties* propertiesToFill)
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
			H_ERROR(StringL::Format("Failed to import ImGui Texture: %", filepath.Object().Name()));
			return nullptr;
		}

		inStream.CloseFile();
		compiledTextureData.loadState = TEXTURE_LOADSTATE::LOADED_TO_RAM;
	}
	else
	{
		//TODO: import external texture 
	}

	VlkDevice& device = *(VlkDevice*)m_device;

	if (propertiesToFill)
		*propertiesToFill = compiledTextureData.properties;

	const uint32_t imageSize = GetTextureByteSize(compiledTextureData.properties);

	// Texture
	// TODO: Cache temporary texture instead of new
	VlkTextureResource* pVlkTexture = (VlkTextureResource*)CreateTextureInternalNoLoad();
	pVlkTexture->textureName = "ImGui TransferTexture";
	pVlkTexture->m_index = MAX_UINT - 1;
	pVlkTexture->m_properties = compiledTextureData.properties;
	pVlkTexture->m_properties.textureUsage = eTextureUsage::Texture;
	H_ASSERT(CreateTextureGPUData(pRenderContext, compiledTextureData, pVlkTexture), "Failed to create default texture");
	pRenderContext->UploadDataToTexture(pVlkTexture, compiledTextureData.compiledColorValues, 0);

	// View
	// TODO: Cache temporary texture view instead of new
	VlkTextureView* pView = (VlkTextureView*)CreateTextureView();
	TextureViewProperties props{};
	props.pTextureToView = pVlkTexture;
	props.viewUsage = eTextureUsage::Texture;
	H_ASSERT(pView->InitView(m_device, props));

	// Move data to imgui texture
	ImGuiVlkTextureResource* returnResource = new ImGuiVlkTextureResource();
	returnResource->metaDataOfResource = metaData;
	returnResource->m_imageView = pView->GetVkImageView();
	returnResource->m_height = compiledTextureData.properties.height;
	returnResource->m_width = compiledTextureData.properties.width;
	returnResource->m_allocation = pVlkTexture->m_textureData.allocation;
	returnResource->m_image = pVlkTexture->m_textureData.textureImage;

	// Create Descriptor Set using ImGUI's implementation
	VlkSamplerObject* vlkSampler = (VlkSamplerObject*)renderingResourceManager->GetGlobalSampler(GlobalSamplers::Point);
	returnResource->m_ImGuiResource = ImGui_ImplVulkan_AddTexture(vlkSampler->GetInternalSampler(), returnResource->m_imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	SAFEDELETE(pVlkTexture);
	SAFEDELETE(pView);

	return returnResource;
}
#pragma optimize("", on)

void Hail::VlkTextureResourceManager::DeleteImGuiTextureResource(ImGuiTextureResource* textureToDelete)
{
	VlkDevice& vlkDevice = *(VlkDevice*)m_device;
	ImGuiVlkTextureResource* textureData = (ImGuiVlkTextureResource*)textureToDelete;
	if (textureData->m_imageView != VK_NULL_HANDLE)
	{
		ImGui_ImplVulkan_RemoveTexture(textureData->m_ImGuiResource);
		vkDestroyImageView(vlkDevice.GetDevice(), textureData->m_imageView, nullptr);
	}
	if (textureData->m_image != VK_NULL_HANDLE)
	{
		vmaDestroyImage(vlkDevice.GetMemoryAllocator(), textureData->m_image, textureData->m_allocation);
	}
	textureData->m_image = VK_NULL_HANDLE;
	textureData->m_allocation = VK_NULL_HANDLE;
	textureData->m_imageView = VK_NULL_HANDLE;
}
