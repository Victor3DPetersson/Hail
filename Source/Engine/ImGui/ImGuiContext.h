#pragma once
#include "Types.h"

#include "Resources_Textures\TextureCommons.h"
#include "Utility\FileSystem.h"
#include "MetaResource.h"
#include "Resources\MaterialResources.h"

namespace Hail
{
	struct ImGuiTextureResource;
	class ResourceManager;

	enum class ImGuiContextsType
	{
		Texture,
		Material,
		Shader,
		None,
	};


	struct MaterialResourceContextObject
	{
		SelectAbleFileObject* m_fileObject;
		SerializeableMaterial m_materialObject;
		MetaResource m_metaResource;
	};

	struct TextureContextAsset
	{
		ImGuiTextureResource* m_texture;
		SelectAbleFileObject m_fileObject;
		FilePath m_filePath;
		TextureProperties m_TextureProperties;

		bool operator==(const TextureContextAsset& other) const;
	};

	struct ShaderResourceContextObject
	{
		SelectAbleFileObject* m_pFileObject;
		MetaResource m_metaResource;
		CompiledShader* m_pShader;
	};

	class ImGuiContext
	{
	public:
		void SetResourceManager(ResourceManager* pResourceManager) { m_pResourceManager = pResourceManager; }
		void SetCurrentContextObject(ImGuiContextsType contextType, void* contextObject);
		void* GetCurrentContextObject();
		ResourceManager* GetResourceManager() { return m_pResourceManager; }
		ImGuiContextsType GetCurrentContextType() const;
		void DeselectContext();
	private:
		ImGuiContextsType m_currentContextType;
		void* m_currentContextObject;
		ResourceManager* m_pResourceManager;
	};
}
