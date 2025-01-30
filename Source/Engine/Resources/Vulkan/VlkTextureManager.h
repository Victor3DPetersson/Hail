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
		explicit VlkTextureResourceManager(RenderingDevice* pDevice);

		FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, eTextureFormat format, TEXTURE_DEPTH_FORMAT depthFormat) override;
		TextureView* CreateTextureView() override;

		ImGuiTextureResource* CreateImGuiTextureResource(const FilePath& filepath, RenderingResourceManager* renderingResourceManager, TextureProperties* headerToFill) override;
		void DeleteImGuiTextureResource(ImGuiTextureResource* textureToDelete) override;

	private:
		TextureResource* CreateTextureInternal(const char* name, CompiledTexture& compiledTextureData) override;
		TextureResource* CreateTextureInternalNoLoad() override;
		bool CreateTextureGPUData(RenderContext* pRenderContext, CompiledTexture& compiledTextureData, TextureResource* pTextureResource) override;

		void ClearTextureInternalForReload(int textureIndex, uint32 frameInFlight) override;
		bool ReloadTextureInternal(int textureIndex, uint32 frameInFlight) override;
		bool CreateTextureData(CompiledTexture& textureData, VlkTextureData& vlkTextureData);
		//VlkTextureResource m_defaultTextureResource;
		//GrowingArray<VlkTextureResource> m_textures;
	};

}