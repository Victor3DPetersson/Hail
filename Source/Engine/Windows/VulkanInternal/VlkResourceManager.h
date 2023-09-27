#pragma once

#include "Resources\TextureManager.h"
#include "Resources\MaterialResources.h"
#include "VlkResources.h"
#include "Rendering\UniformBufferManager.h"
#include "Resources\Vulkan\VlkTextureResource.h"

namespace Hail
{
	struct CompiledTexture;
	class RenderingDevice;
	class VlkDevice;
	class VlkFrameBufferTexture;
	class VlkSwapChain;

	class VlkTextureResourceManager : public TextureManager
	{
	public:
		void Init(RenderingDevice* device) final;
		void ClearAllResources() final;
		bool LoadTexture(const char* textureName) final;

		VlkTextureData& GetTextureData(uint32_t index);
		FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat) final;

	private:
		bool CreateTextureData(CompiledTexture& textureData, VlkTextureData& vlkTextureData);
		VlkDevice* m_device;

		GrowingArray<VlkTextureResource> m_textures;
	};

	class VlkMaterialeResourceManager
	{
	public:
		bool Init(RenderingDevice* device, TextureManager* textureResourceManager, VlkSwapChain* swapChain);
		void ClearAllResources();
		bool InitMaterial(Material& material, FrameBufferTexture* frameBufferTexture);
		bool InitInstance(const Material material, MaterialInstance& instance);

		VlkPassData& GetMaterialData(MATERIAL_TYPE material);
		void MapMemoryToBuffer(BUFFERS buffer, void* dataToMap, uint32_t sizeOfData);
		VkDescriptorSet& GetGlobalDescriptorSet(uint32_t frameInFlight);

	private:
		bool CreateMaterialPipeline(Material& material);
		bool SetUpCommonLayouts();
		bool SetUpMaterialLayouts(VlkPassData& passData, MATERIAL_TYPE type);
		bool CreateRenderpassAndFramebuffers(VlkPassData& passData, MATERIAL_TYPE type);

		VlkSwapChain* m_swapChain = nullptr;
		VlkDevice* m_device = nullptr;
		VlkTextureResourceManager* m_textureResourceManager = nullptr;
		VlkPassData m_passData[(uint32)(MATERIAL_TYPE::COUNT)];


		VkDescriptorPool m_globalDescriptorPool = VK_NULL_HANDLE;

		VkDescriptorSetLayout m_globalPerFrameSetLayout = VK_NULL_HANDLE;
		VkDescriptorSet m_globalDescriptorSetsPerFrame[MAX_FRAMESINFLIGHT];


		VkSampler m_linearTextureSampler = VK_NULL_HANDLE;
		VkSampler m_pointTextureSampler = VK_NULL_HANDLE;

		VlkBufferObject m_buffers[(uint32)(BUFFERS::COUNT)];

		VlkFrameBufferTexture* m_passesFrameBufferTextures[static_cast<uint32_t>(MATERIAL_TYPE::COUNT)];
		//This is here so the local variables does not get optimized away in release
		VkDescriptorImageInfo m_descriptorImageInfo;
		VkDescriptorBufferInfo m_descriptorBufferInfo;
	};

}

