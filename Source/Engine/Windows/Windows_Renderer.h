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
	class VlkSwapChain;
	class VlkRenderer : public Renderer
	{
	public:
		bool InitDevice(RESOLUTIONS startupResolution, Timer* timer) override;
		bool InitGraphicsEngine(ResourceManager* resourceManager) override;
		void StartFrame(RenderCommandPool& renderPool) final;
		void Render() final;
		void EndFrame() final;
		void Cleanup() final;
		void InitImGui() final;

		void BindMaterial(Material& materialToBind) override;
		void EndMaterialPass() override;
		void RenderSprite(const RenderCommand_Sprite& spriteCommandToRender, uint32_t spriteInstance) override;
		void RenderMesh(const RenderCommand_Mesh& meshCommandToRender, uint32_t meshInstance) override;
		void RenderLetterBoxPass() override;

	private:

		void CreateCommandBuffers();
		void CreateSyncObjects();

		void CreateVertexBuffer();
		void CreateFullscreenVertexBuffer();
		void CreateSpriteVertexBuffer();
		void CreateIndexBuffer();

	
		VlkSwapChain* m_swapChain = nullptr;

		VkCommandBuffer m_commandBuffers[MAX_FRAMESINFLIGHT];

		
		VkBuffer m_fullscreenVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_fullscreenVertexBufferMemory = VK_NULL_HANDLE;

		VkBuffer m_spriteVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_spriteVertexBufferMemory = VK_NULL_HANDLE;

		//Vertex and index buffer for cube
		VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer m_indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;


		MATERIAL_TYPE m_boundMaterialType = MATERIAL_TYPE::COUNT;
		bool m_commandBufferBound = false;

		VkDescriptorPool m_imguiPool = VK_NULL_HANDLE;

		VkSemaphore m_imageAvailableSemaphores[MAX_FRAMESINFLIGHT];
		VkSemaphore m_renderFinishedSemaphores[MAX_FRAMESINFLIGHT];
		VkFence m_inFrameFences[MAX_FRAMESINFLIGHT];
	};
}

