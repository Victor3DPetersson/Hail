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
		bool InitDevice(Timer* timer) override;
		bool InitGraphicsEngine(ResourceManager* resourceManager) override;
		void StartFrame(RenderCommandPool& renderPool) override;
		void Render() override;
		void EndFrame() override;
		void Cleanup() override;
		void InitImGui() override;
		void WaitForGPU() override;

		void BindMaterial(Material& materialToBind, bool bFirstMaterialInFrame) override;
		void EndMaterialPass() override;
		void RenderSprite(const RenderCommand_Sprite& spriteCommandToRender, uint32_t spriteInstance) override;
		void RenderMesh(const RenderCommand_Mesh& meshCommandToRender, uint32_t meshInstance) override;
		void RenderDebugLines2D(uint32 numberOfLinesToRender, uint32 offsetFrom3DLines) override;
		void RenderDebugLines3D(uint32 numberOfLinesToRender) override;
		void RenderLetterBoxPass() override;

	private:

		void CreateCommandBuffers();
		void CreateSyncObjects();

		void CreateVertexBuffer();
		void CreateFullscreenVertexBuffer();
		void CreateSpriteVertexBuffer();
		void CreateIndexBuffer();
		void CreateDebugLineVertexBuffer();

	
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

		//Debug line buffers
		VkBuffer m_debugLineVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_debugLineVertexBufferMemory = VK_NULL_HANDLE;

		eMaterialType m_boundMaterialType = eMaterialType::COUNT;
		bool m_commandBufferBound = false;

		VkDescriptorPool m_imguiPool = VK_NULL_HANDLE;

		VkSemaphore m_imageAvailableSemaphores[MAX_FRAMESINFLIGHT];
		VkSemaphore m_renderFinishedSemaphores[MAX_FRAMESINFLIGHT];
		VkFence m_inFrameFences[MAX_FRAMESINFLIGHT];
	};
}

