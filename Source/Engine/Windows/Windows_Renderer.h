//Interface for the entire engine
#pragma once

#include "Renderer.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan\vulkan.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "VulkanInternal\VlkDevice.h"
#include "VulkanInternal\VlkTextureCreationFunctions.h"

namespace Hail
{
	class VlkBufferObject;
	class VlkSwapChain;
	class VlkRenderer : public Renderer 
	{
	public:
		bool Initialize() override;
		bool InitDevice(Timer* timer) override;
		bool InitGraphicsEngineAndContext(ResourceManager* resourceManager) override;
		void StartFrame(RenderCommandPool& renderPool) override;
		void Render() override;
		void Cleanup() override;
		void InitImGui() override;
		void WaitForGPU() override;

		void RenderSprite(const RenderCommand_Sprite& spriteCommandToRender, uint32_t spriteInstance) override;
		void RenderMesh(const RenderCommand_Mesh& meshCommandToRender, uint32_t meshInstance) override;
		void RenderDebugLines2D(uint32 numberOfLinesToRender, uint32 offsetFrom3DLines) override;
		void RenderDebugLines3D(uint32 numberOfLinesToRender) override;
		void RenderLetterBoxPass() override;

	private:

		// TODO: move out of the Vulkan renderer and have it in the main renderer.
		void CreateVertexBuffer();
		void CreateFullscreenVertexBuffer();
		void CreateSpriteVertexBuffer();
		void CreateIndexBuffer();
		void CreateDebugLineVertexBuffer();

		VlkBufferObject* m_pFullscreenVertexBuffer = nullptr;
		VlkBufferObject* m_pSpriteVertexBuffer = nullptr;
		VlkBufferObject* m_pVertexBuffer = nullptr;
		VlkBufferObject* m_pIndexBuffer = nullptr;
		VlkBufferObject* m_pDebugLineVertexBuffer = nullptr;
	
		VlkSwapChain* m_swapChain = nullptr;

		VkDescriptorPool m_imguiPool = VK_NULL_HANDLE;
	};
}

