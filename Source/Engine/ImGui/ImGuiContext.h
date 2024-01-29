#pragma once
#include "Types.h"

#include "Resources\TextureCommons.h"
#include "Utility\FileSystem.h"
#include "Resources\MetaResource.h"
#include "Resources\MaterialResources.h"

namespace Hail
{
	struct ImGuiTextureResource;

	enum class ImGuiContextsType
	{
		Texture,
		Material,
		None,
	};


	struct MaterialResourceContextObject
	{
		SelectAbleFileObject* m_fileObject;
		SerializeableMaterialInstance m_materialObject;
		MetaResource m_metaResource;
	};

	struct TextureContextAsset
	{
		ImGuiTextureResource* m_texture;
		SelectAbleFileObject m_fileObject;
		TextureHeader m_textureHeader;

		bool operator==(const TextureContextAsset& other) const;
	};

	class ImGuiContext
	{
	public:
		void SetCurrentContextObject(ImGuiContextsType contextType, void* contextObject);
		void* GetCurrentContextObject();
		ImGuiContextsType GetCurrentContextType() const;
		void DeselectContext();
	private:
		ImGuiContextsType m_currentContextType;
		void* m_currentContextObject;
	};
}
