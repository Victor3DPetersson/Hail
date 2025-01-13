#pragma once

#include "Resources\MaterialResources.h"
#include "Resources\RenderingResourceManager.h"
#include "VlkResources.h"

namespace Hail
{
	struct CompiledTexture;
	class RenderingDevice;
	class VlkDevice;
	class VlkFrameBufferTexture;
	class VlkSwapChain;
	class TextureManager;
	class VlkTextureResourceManager;

	struct VlkRenderingResources
	{
		VkDescriptorPool m_globalDescriptorPool = VK_NULL_HANDLE;
		//VkDescriptorSetLayout m_globalPerFrameSetLayout = VK_NULL_HANDLE;
		//VkDescriptorSet m_globalDescriptorSetsPerFrame[MAX_FRAMESINFLIGHT];

		VkSampler m_linearTextureSampler = VK_NULL_HANDLE;
		VkSampler m_pointTextureSampler = VK_NULL_HANDLE;

		//VlkBufferObject m_buffers[(uint32)(BUFFERS::COUNT)];
	};


	class VlkRenderingResourceManager : public RenderingResourceManager
	{
	public:
		bool Init(RenderingDevice* renderingDevice, SwapChain* swapChain) override;
		void ClearAllResources() override;

		void* GetRenderingResources() override;

		void UploadMemoryToBuffer(BufferObject* buffer, void* dataToMap, uint32 sizeOfData, uint32 offset = 0) override;

		BufferObject* CreateBuffer(BufferProperties properties, eDecorationSets setToCreateBufferFor) override;


	private:

		VlkRenderingResources m_resources;
	};

}

