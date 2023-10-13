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
		bool LoadTexture(const char* textureName) final;
		VlkTextureData& GetTextureData(uint32_t index);
		FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat) final;

	private:
		void ClearTextureInternal(int textureIndex) final;
		bool ReloadTextureInternal(int textureIndex) final;
		bool CreateTextureData(CompiledTexture& textureData, VlkTextureData& vlkTextureData);
		VlkDevice* m_device;
		GrowingArray<VlkTextureResource> m_textures;
	};

}