//Interface for the entire engine
#pragma once

#include "VlkTextureCreationFunctions.h"
#include "Rendering\FrameBufferTexture.h"

namespace Hail
{
	class VlkFrameBufferTexture : public FrameBufferTexture
	{
		friend class VlkSwapChain;
	public:
		VlkFrameBufferTexture(glm::uvec2 resolution, eTextureFormat format = eTextureFormat::UNDEFINED, TEXTURE_DEPTH_FORMAT depthFormat = TEXTURE_DEPTH_FORMAT::UNDEFINED);

		void CreateFrameBufferTextureObjects(RenderingDevice* pDevice) override;

		void ClearResources(RenderingDevice* device, bool isSwapchain = false) override;

		VkRenderPass GetVkRenderPass() { return m_renderPass; }
		VkFramebuffer GetVkFrameBuffer(uint32 aFrameInFlight) { return m_frameBuffers[aFrameInFlight]; }

	private:
		void CreateTextureResources(bool isColorTexture, RenderingDevice* pDevice)  override;
		bool CreateFramebuffers(RenderingDevice* pDevice, uint32 frameInFlight);
		bool CreateRenderpass(RenderingDevice* pDevice);

		VkRenderPass m_renderPass;
		StaticArray<VkFramebuffer, MAX_FRAMESINFLIGHT > m_frameBuffers;
	};
}