#pragma once

#include "Rendering\UniformBufferManager.h"

namespace Hail
{
	class RenderingDevice;
	class TextureManager;
	class SwapChain;


	//Class that holds non material and non texture resources, so buffers, samplers and the like
	class RenderingResourceManager
	{
	public:

		virtual bool Init(RenderingDevice* renderingDevice, SwapChain* swapChain) = 0;

		//overloaded function that gets the udnerlying resources from the platform version
		virtual void* GetRenderingResources() = 0;

		virtual void ClearAllResources() = 0;
		virtual void MapMemoryToBuffer(BUFFERS buffer, void* dataToMap, uint32_t sizeOfData) = 0;

	protected:

		RenderingDevice* m_renderDevice;
		SwapChain* m_swapChain;
	};



}