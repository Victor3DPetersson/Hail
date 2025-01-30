#pragma once
#include "../EngineConstants.h"
#include "Resources_Textures\TextureCommons.h"
#include "ResourceCommon.h"

#include "MetaResource.h"

namespace Hail
{
	class RenderingDevice;
	class TextureManager;


	class TextureResource
	{
	public:
		virtual void CleanupResource(RenderingDevice* device) = 0;
		virtual void CleanupResourceForReload(RenderingDevice* device, uint32 frameInFligth) = 0;

		bool Init(RenderingDevice* pDevice);

		String64 textureName;
		uint32_t m_index = MAX_UINT;
		CompiledTexture m_compiledTextureData;
		TextureProperties m_properties;
		ResourceValidator m_validator;
		MetaResource m_metaResource;

	protected: 

		friend class TextureManager;

		virtual bool InternalInit(RenderingDevice* pDevice) = 0;

		static uint32 g_idCounter;
	};

	struct TextureViewProperties
	{
		TextureResource* pTextureToView;
		eTextureUsage viewUsage;
	};

	class TextureView
	{
	public:
		virtual void CleanupResource(RenderingDevice* pDevice) = 0;

		virtual bool InitView(RenderingDevice* pDevice, TextureViewProperties properties) = 0;
	protected:
		uint32 m_textureIndex{ MAX_UINT };
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

