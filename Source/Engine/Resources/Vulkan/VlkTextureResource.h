#pragma once
#include "..\TextureResource.h"
#include "Windows\VulkanInternal\VlkResources.h"

namespace Hail
{
	class RenderingDevice;
	class VlkTextureResource
	{
	public:

		void CleanupResource(RenderingDevice* device);
		void CleanupResourceForReload(RenderingDevice* device, uint32 frameInFligth);
		VlkTextureData& GetVlkTextureData() { return m_textureData; }
		ResourceValidator& GetValidator() { return m_validator; }
	private:
		VlkTextureData m_textureData;
		VlkTextureData m_unloadingTextureData;
		ResourceValidator m_validator = ResourceValidator();
	};
}