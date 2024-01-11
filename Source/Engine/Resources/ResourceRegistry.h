#pragma once

#include "Types.h"
#include "MetaResource.h"
#include "Containers\GrowingArray\GrowingArray.h"

namespace Hail
{
	//TODO: Add more once more resources get added to engine
	enum class ResourceType
	{
		Texture
	};

	class ResourceRegistry
	{
	public:


		void Init();
		void AddToRegistry(const FilePath& projectPath, ResourceType type);

		FilePath GetProjectPath(ResourceType type, GUID resourceGuid) const;
		bool GetIsResourceImported(ResourceType type, GUID resourceGuid) const;
		bool GetIsResourceLoaded(ResourceType type, GUID resourceGuid) const;
		void SetResourceLoaded(ResourceType type, GUID resourceGuid);
		void SetResourceUnloaded(ResourceType type, GUID resourceGuid);

	private:
		struct MetaDataWithKey
		{
			GUID key;
			RelativeFilePath projectPath;
			bool bIsLoaded;
		};

		//TODO: use a red black tree structure or hash these values later
		//TODO: add more types of resources here
		GrowingArray<MetaDataWithKey> m_textureResources;



	};
}