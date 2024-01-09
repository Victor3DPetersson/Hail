#pragma once
#include "../EngineConstants.h"
#include "TextureCommons.h"

#include "MetaResource.h"

namespace Hail
{
	class RenderingDevice;
	class TextureResource
	{
	public:

	//private:
		String64 textureName;
		uint32_t index = 0;
		GUID m_uuid;
		CompiledTexture m_compiledTextureData;
	};

	// Only use for ImGui data
	class ImGuiTextureResource
	{
	public:

		virtual void* GetImguiTextureResource() = 0;

	
		uint32 m_width = 0;
		uint32 m_height = 0;
		String64 textureName;
		MetaResource metaDataOfResource;
	};

}

