#pragma once

#include "EngineConstants.h"
#include "Resource.h"
#include "MeshResources.h"
#include "Containers\VectorOnStack\VectorOnStack.h"
#include "Rendering\UniformBufferManager.h"

class Timer;

namespace Hail
{
	struct RenderCommandPool;
	class FrameBufferTexture;
	class MaterialManager;
	class RenderingDevice;
	class RenderingResourceManager;
	class SwapChain;
	class TextureManager;

	class ResourceManager
	{
	public:
		ResourceManager();
		bool InitResources(RenderingDevice* renderingDevice);
		void ClearAllResources();
		MaterialManager* GetMaterialManager() { return m_materialManager; }
		TextureManager* GetTextureManager() { return m_textureManager; }
		RenderingResourceManager* GetRenderingResourceManager() { return m_renderingResourceManager; }
		void SetTargetResolution(glm::uvec2 targetResolution);
		void SetReloadOfAllTextures() { m_reloadTextures = true; }
		void ReloadResources();

		void LoadMaterial(GUID guid);
		void LoadMaterial(String256 name);

		void LoadTextureResource(GUID guid);
		void LoadTextureResource(String256 name);
		void LoadMaterialResource(GUID guid);
		void LoadMaterialResource(String256 name);

		void UpdateRenderBuffers(RenderCommandPool& renderPool, Timer* timer);
		void ClearFrameData();
		void SetSwapchainTargetResolution(glm::uvec2 targetResolution);

		SwapChain* GetSwapChain() { return m_swapChain; }

		Mesh m_unitCube;
		Mesh m_unitSphere;
		Mesh m_unitCylinder;

	private:

		MaterialManager* m_materialManager = nullptr;
		TextureManager* m_textureManager = nullptr;
		RenderingDevice* m_renderDevice = nullptr;

		SwapChain* m_swapChain = nullptr;
		RenderingResourceManager* m_renderingResourceManager = nullptr;
		FrameBufferTexture* m_mainPassFrameBufferTexture = nullptr;


		VectorOnStack<SpriteInstanceData, MAX_NUMBER_OF_SPRITES, false> m_spriteInstanceData;
		bool m_reloadTextures = false;
	};
}

