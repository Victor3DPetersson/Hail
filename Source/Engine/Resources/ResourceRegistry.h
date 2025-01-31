#pragma once

#include "Types.h"
#include "MetaResource.h"
#include "Containers\GrowingArray\GrowingArray.h"

namespace Hail
{
	//TODO: Add more once more resources get added to engine
	enum class ResourceType
	{
		Texture,
		Material,
		Shader
	};

	class ResourceRegistry
	{
	public:
		void Init();
		void AddToRegistry(const FilePath& resourcePath, ResourceType type);

		FilePath GetProjectPath(ResourceType type, GUID resourceGuid) const;
		FilePath GetSourcePath(ResourceType type, GUID resourceGuid) const;
		// A resource is imported if it exists in the folder structure.
		bool GetIsResourceImported(ResourceType type, GUID resourceGuid) const;
		// A resource is loaded if it has been opened and processed by the appropriate manager.
		bool GetIsResourceLoaded(ResourceType type, GUID resourceGuid) const;
		// To be able to query the state manmually
		eResourceState GetResourceState(ResourceType type, GUID resourceGuid) const;
		void SetResourceLoaded(ResourceType type, GUID resourceGuid);
		void SetResourceUnloaded(ResourceType type, GUID resourceGuid);
		void SetResourceLoadFailed(ResourceType type, GUID resourceGuid);
		String64 GetResourceName(ResourceType type, GUID resourceGUID) const;
		// Checks the source resource and compares it with the project resource to see if it needs to be reloaded. Does not work with all types.
		bool IsResourceOutOfDate(ResourceType type, GUID resourceGUID);

		const MetaResource* GetResourceMetaInformation(ResourceType type, const FilePath& pathToCheck) const;

	private:
		struct MetaData
		{
			MetaResource m_resource;
			eResourceState m_state;
		};
		FilePath GetProjectFilePathInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const;
		FilePath GetSourceFilePathInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const;
		bool GetIsResourceImportedInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const;
		bool GetIsResourceLoadedInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const;
		bool IsResourceOutOfDateInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const;
		void SetIsResourceLoadedInternal(GrowingArray<MetaData>& list, const GUID& resourceGuid);
		void SetIsResourceUnloadedInternal(GrowingArray<MetaData>& list, const GUID& resourceGuid);
		void SetIsResourceLoadFailedInternal(GrowingArray<MetaData>& list, const GUID& resourceGuid);
		String64 GetResourceNameInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const;
		const MetaResource* GetMetaResourceInternal(const GrowingArray<MetaData>& list, const FilePath& pathToCheck) const;
		eResourceState GetResourceStateInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const;
		//TODO: use a red black tree structure or hash these values later
		//TODO: add more types of resources here
		GrowingArray<MetaData> m_textureResources;
		GrowingArray<MetaData> m_materialResources;
		GrowingArray<MetaData> m_shaderResources;


	};
}