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
		void SetReloadOfAllResources();
		void SetReloadOfAllTextures();
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

		//Dependency
		RenderingDevice* m_renderDevice = nullptr;

		//Resource managers
		RenderingResourceManager* m_renderingResourceManager = nullptr;
		MaterialManager* m_materialManager = nullptr;
		TextureManager* m_textureManager = nullptr;
		SwapChain* m_swapChain = nullptr;

		FrameBufferTexture* m_mainPassFrameBufferTexture = nullptr;

		//Instance resources

		VectorOnStack<SpriteInstanceData, MAX_NUMBER_OF_SPRITES, false> m_spriteInstanceData;


		//Reloading
		
		bool m_reloadEverything = false;
		bool m_reloadAllTextures = false;
		uint32 m_frameInFlightIndex = 0;
		uint32 m_reloadFrameCounter = 0;


	};
}

