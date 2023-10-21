#pragma once
#include "Resources\TextureManager.h"
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
		VlkTextureData& GetTextureData(uint32_t index);
		VlkTextureResource& GetDefaultTextureData() { return m_defaultTextureResource; }

		FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat) final;

	private:
		bool CreateTextureInternal(TextureResource& textureToCreate, bool createDefaultTexture) final;
		void ClearTextureInternalForReload(int textureIndex, uint32 frameInFlight) final;
		bool ReloadTextureInternal(int textureIndex, uint32 frameInFlight) final;
		bool CreateTextureData(CompiledTexture& textureData, VlkTextureData& vlkTextureData);
		VlkDevice* m_device;
		VlkTextureResource m_defaultTextureResource;
		GrowingArray<VlkTextureResource> m_textures;
	};

}