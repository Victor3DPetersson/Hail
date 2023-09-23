#pragma once

#include "MeshResources.h"
#include "TextureManager.h"
#include "MaterialManager.h"
#include "Containers\VectorOnStack\VectorOnStack.h"
#ifdef PLATFORM_WINDOWS
#include "windows/VulkanInternal/VlkResourceManager.h"
#include "windows/VulkanInternal/VlkSwapChain.h"
#endif

class Timer;

namespace Hail
{
	struct RenderCommandPool;
	class RenderingDevice;
	class ResourceManager
	{
	public:
		ResourceManager();
		bool InitResources(RenderingDevice* renderingDevice);
		void ClearAllResources();
		MaterialManager& GetMaterialManager() { return m_materialManager; }
		TextureManager& GetTextureManager() { return m_textureManager; }

		void SetTargetResolution(glm::uvec2 targetResolution);


		void LoadMaterial(GUID guid);
		void LoadMaterial(String256 name);

		void LoadTextureResource(GUID guid);
		void LoadTextureResource(String256 name);
		void LoadMaterialResource(GUID guid);
		void LoadMaterialResource(String256 name);

		FrameBufferTexture* FrameBufferTexture_Create(String64 name, glm::uvec2 resolution, TEXTURE_FORMAT format, TEXTURE_DEPTH_FORMAT depthFormat);
		
		void UpdateRenderBuffers(RenderCommandPool& renderPool, Timer* timer);
		void ClearFrameData();
		void SetSwapchainTargetResolution(glm::uvec2 targetResolution);

#ifdef PLATFORM_WINDOWS
		VlkTextureResourceManager& GetVulkanTextureResources() { return m_platformTextureResourceManager; }
		VlkMaterialeResourceManager& GetVulkanMaterialResources() { return m_platformMaterialResourceManager; }
		VlkSwapChain& GetVulkanSwapChain() { return m_platformSwapChain; }
#endif

		Mesh m_unitCube;
		Mesh m_unitSphere;
		Mesh m_unitCylinder;

	private:

		MaterialManager m_materialManager;
		TextureManager m_textureManager;
		RenderingDevice* m_renderDevice;

#ifdef PLATFORM_WINDOWS
		VlkTextureResourceManager m_platformTextureResourceManager;
		VlkMaterialeResourceManager m_platformMaterialResourceManager;
		VlkSwapChain m_platformSwapChain;
#endif
		FrameBufferTexture* m_mainPassFrameBufferTexture = nullptr;


		VectorOnStack<SpriteInstanceData, MAX_NUMBER_OF_SPRITES, false> m_spriteInstanceData;

	};
}

