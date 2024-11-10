#pragma once
#include "../EngineConstants.h"
#include "Resources_Textures\TextureCommons.h"
#include "ResourceCommon.h"

#include "MetaResource.h"

namespace Hail
{
	class RenderingDevice;
	class TextureResource
	{
	public:
		virtual void CleanupResource(RenderingDevice* device) = 0;
		virtual void CleanupResourceForReload(RenderingDevice* device, uint32 frameInFligth) = 0;

		String64 textureName;
		uint32_t m_index;
		CompiledTexture m_compiledTextureData;
		ResourceValidator m_validator;
		MetaResource m_metaResource;

		static uint32 g_idCounter;
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

