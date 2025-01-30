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

		VkSampler m_linearTextureSampler = VK_NULL_HANDLE;
		VkSampler m_pointTextureSampler = VK_NULL_HANDLE;

	};


	class VlkRenderingResourceManager : public RenderingResourceManager
	{
	public:
		bool Init(RenderingDevice* renderingDevice, SwapChain* swapChain) override;
		void ClearAllResources() override;

		void* GetRenderingResources() override;

		BufferObject* CreateBuffer(BufferProperties properties) override;

	private:

		VlkRenderingResources m_resources;
	};

}

