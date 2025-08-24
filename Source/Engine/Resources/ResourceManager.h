#pragma once

#include "EngineConstants.h"
#include "ResourceCommon.h"
#include "MeshResources.h"
#include "BufferResource.h"
#include "Containers\VectorOnStack\VectorOnStack.h"

namespace Hail
{
	struct RenderCommandPool;
	class ErrorManager;
	class FrameBufferTexture;
	class MaterialManager;
	class RenderContext;
	class RenderingDevice;
	class RenderingResourceManager;
	class SwapChain;
	class TextureManager;
	class Timer;
	struct GameCommand_Sprite;
	struct RenderCommand2DBase;
	struct RenderData_Sprite;

	class ResourceManager
	{
	public:
		ResourceManager();
		bool InitResources(RenderingDevice* renderingDevice, RenderContext* pRenderContext, eResolutions targetRes, eResolutions startupWindowRes, ErrorManager* pErrorManager);
		void ClearAllResources(RenderingDevice* pRenderDevice);
		MaterialManager* GetMaterialManager() { return m_materialManager; }
		TextureManager* GetTextureManager() { return m_textureManager; }
		RenderingResourceManager* GetRenderingResourceManager() { return m_renderingResourceManager; }
		void SetTargetResolution(eResolutions targetResolution);
		void SetWindowResolution(eResolutions targetResolution);
		eResolutions GetTargetResolution() const { return m_targetResolution; }
		void SetReloadOfAllResources();
		void SetReloadOfAllTextures();

		void ReloadResources();

		// TODO: Move frame buffer and what owns it to a different place
		FrameBufferTexture* GetMainPassFBTexture() { return m_pMainPassFrameBufferTexture; }

		void LoadMaterialResource(GUID guid);
		uint32 GetMaterialInstanceHandle(GUID guid) const;

		void UpdateRenderBuffers(RenderCommandPool& renderPool, RenderContext* pRenderContext, Timer* timer);
		void ClearFrameData();
		void SetSwapchainTargetResolution(glm::uvec2 targetResolution);

		void SpriteRenderDataFromGameCommand(const GameCommand_Sprite& commandToCreateFrom, RenderCommand2DBase& baseCommandToFill, RenderData_Sprite& dataToFill);
		
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

		// Resources

		FrameBufferTexture* m_pMainPassFrameBufferTexture = nullptr;

		//Reloading
		
		bool m_reloadEverything = false;
		bool m_reloadAllTextures = false;
		uint32 m_frameInFlightIndex = 0;
		uint32 m_reloadFrameCounter = 0;


	};
}

