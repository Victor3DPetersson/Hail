#pragma once

#include "Resources\MaterialResources.h"
#include "Resources\RenderingResourceManager.h"
#include "Rendering\UniformBufferManager.h"
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
		VkDescriptorSetLayout m_globalPerFrameSetLayout = VK_NULL_HANDLE;
		VkDescriptorSet m_globalDescriptorSetsPerFrame[MAX_FRAMESINFLIGHT];

		VkSampler m_linearTextureSampler = VK_NULL_HANDLE;
		VkSampler m_pointTextureSampler = VK_NULL_HANDLE;

		VlkBufferObject m_buffers[(uint32)(BUFFERS::COUNT)];
	};

	class VlkRenderingResourceManager : public RenderingResourceManager
	{
	public:
		bool Init(RenderingDevice* renderingDevice, SwapChain* swapChain) final;
		void ClearAllResources() final;

		void* GetRenderingResources() final;

		void MapMemoryToBuffer(BUFFERS buffer, void* dataToMap, uint32_t sizeOfData) final;

		VkDescriptorSet& GetGlobalDescriptorSet(uint32_t frameInFlight);

	private:
		bool SetUpCommonLayouts();

		VlkRenderingResources m_resources;
	};

}

