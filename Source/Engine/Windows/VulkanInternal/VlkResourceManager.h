#pragma once

#include "Resources\MaterialResources.h"
#include "VlkResources.h"


namespace Hail
{
	struct CompiledTexture;
	class RenderingDevice;
	class VlkDevice;
	class VlkResourceManager
	{
	public:
		void Init(RenderingDevice* device);
		void CreateMaterialDescriptor();
		bool CreateTextureData(CompiledTexture& textureData );

		VlkMaterialDescriptor& GetMaterialData(uint32_t index);
		VlkTextureData& GetTextureData(uint32_t index);


	private:
		VlkDevice* m_device;

		GrowingArray<VlkMaterialDescriptor> m_materialDescriptors;
		GrowingArray<VlkTextureData> m_textureData;
	};

}

