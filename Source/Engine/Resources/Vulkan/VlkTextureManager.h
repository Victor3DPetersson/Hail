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
		void Init(RenderingDevice* device) override;

		FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat) override;

		ImGuiTextureResource* CreateImGuiTextureResource(const FilePath& filepath, RenderingResourceManager* renderingResourceManager, TextureHeader* headerToFill) override;
		void DeleteImGuiTextureResource(ImGuiTextureResource* textureToDelete) override;

	private:
		TextureResource* CreateTextureInternal(const char* name, CompiledTexture& compiledTextureData) override;
		void ClearTextureInternalForReload(int textureIndex, uint32 frameInFlight) override;
		bool ReloadTextureInternal(int textureIndex, uint32 frameInFlight) override;
		bool CreateTextureData(CompiledTexture& textureData, VlkTextureData& vlkTextureData);
		VlkDevice* m_device;
		//VlkTextureResource m_defaultTextureResource;
		//GrowingArray<VlkTextureResource> m_textures;
	};

}