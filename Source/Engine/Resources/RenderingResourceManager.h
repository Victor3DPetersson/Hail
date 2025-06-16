#pragma once

#include "BufferResource.h"
#include "Resources_Materials\Materials_Common.h"
#include "Resources_Textures\TextureCommons.h"

namespace Hail
{
	class RenderingDevice;
	class TextureManager;
	class SwapChain;
	class SamplerObject;

	enum class GlobalSamplers
	{
		Point,
		Bilinear,
		Count
	};

	//Class that holds non material and non texture resources, so buffers, samplers and the like
	class RenderingResourceManager
	{
	public:

		// Creates the internal buffers
		virtual bool Init(RenderingDevice* renderingDevice, SwapChain* swapChain) = 0;
		//overloaded function that gets the udnerlying resources from the platform version
		virtual void* GetRenderingResources() = 0;

		virtual void ClearAllResources() = 0;

		BufferObject* GetGlobalBuffer(eDecorationSets setToGet, eBufferType bufferType, uint8 bindingPoint);
		SamplerObject* GetGlobalSampler(GlobalSamplers samplerToGet);
		virtual BufferObject* CreateBuffer(BufferProperties properties, const char* name) = 0;
		virtual SamplerObject* CreateSamplerObject(SamplerProperties properties) = 0;

	protected:
		// Creates the common buffers, gets called from the virtual init function
		bool InternalInit();

		// 1 for Array for each set, so global and material domains
		StaticArray<GrowingArray<BufferObject*>, 2> m_uniformBuffers;
		StaticArray<GrowingArray<BufferObject*>, 2> m_structuredBuffers;

		// TODO create more sampler objects
		StaticArray<SamplerObject*, (uint32)GlobalSamplers::Count> m_samplers;

		RenderingDevice* m_renderDevice;
		SwapChain* m_swapChain;
	};



}