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
		VlkTextureData& GetVlkTextureData() { return m_textureData; }

	private:
		VlkTextureData m_textureData;
	};
}