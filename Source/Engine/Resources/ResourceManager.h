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
		void SetTargetResolution(eResolutions targetResolution);
		void SetWindowResolution(eResolutions targetResolution);
		eResolutions GetTargetResolution() const { return m_targetResolution; }
		void SetReloadOfAllResources();
		void SetReloadOfAllTextures();
		void ReloadResources();


		void LoadMaterialResource(GUID guid);
		uint32 GetMaterialInstanceHandle(GUID guid) const;

		void UpdateRenderBuffers(RenderCommandPool& renderPool, Timer* timer);
		void ClearFrameData();
		void SetSwapchainTargetResolution(glm::uvec2 targetResolution);

		SwapChain* GetSwapChain() { return m_swapChain; }

		Mesh m_unitCube;
		Mesh m_unitSphere;
		Mesh m_unitCylinder;

	private:

		eResolutions m_targetResolution;

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

		uint32 m_numberOf2DDebugLines;
		uint32 m_numberOf3DDebugLines;
		VectorOnStack<DebugLineData, MAX_NUMBER_OF_DEBUG_LINES, false> m_debugLineData;


		//Reloading
		
		bool m_reloadEverything = false;
		bool m_reloadAllTextures = false;
		uint32 m_frameInFlightIndex = 0;
		uint32 m_reloadFrameCounter = 0;


	};
}

