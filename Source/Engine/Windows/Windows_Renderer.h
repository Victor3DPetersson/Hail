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
		void Initialize(ErrorManager* pErrorManager) override;
		void InitDevice(Timer* pTimer, ErrorManager* pErrorManager) override;
		void InitGraphicsEngineAndContext(ResourceManager* resourceManager) override;
		void StartFrame(RenderCommandPool& renderPool) override;
		void Render() override;
		void Cleanup() override;
		void InitImGui() override;
		void WaitForGPU() override;

		void RenderMesh(const RenderData_Mesh& meshCommandToRender, uint32_t meshInstance) override;
		void RenderImGui() override;

	private:
	
		VlkSwapChain* m_swapChain = nullptr;

		VkDescriptorPool m_imguiPool = VK_NULL_HANDLE;
	};
}

