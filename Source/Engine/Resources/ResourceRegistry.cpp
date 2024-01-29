#include "Engine_PCH.h"

#include "ResourceRegistry.h"
#include "Utility\FileSystem.h"
#include "MaterialManager.h"
#include "TextureManager.h"
using namespace Hail;

namespace
{
}

void ResourceRegistry::Init()
{
	m_textureResources.Init(16);
	m_materialResources.Init(16);
	RecursiveFileIterator fileIterator(FilePath::GetCurrentWorkingDirectory());

	
	while (fileIterator.IterateOverFolderRecursively())
	{
		FilePath currentFilePath = fileIterator.GetCurrentPath();
		const FileObject& currentFileObject = currentFilePath.Object();

		MetaDataWithKey resource;
		resource.bIsLoaded = false;

		if (StringCompare(currentFileObject.Extension(), L"txr"))
		{
			MetaResource resourceToFill;
			TextureManager::LoadTextureMetaData(currentFilePath, resourceToFill);

			if (resourceToFill.GetGUID() == GUID())
				continue;


			resource.key = resourceToFill.GetGUID();
			resource.projectPath = RelativeFilePath(currentFilePath);
			m_textureResources.Add(resource);
		}
		if (StringCompare(currentFileObject.Extension(), L"mat"))
		{
			MetaResource resourceToFill;
			MaterialManager::LoadMaterialMetaData(currentFilePath, resourceToFill);
			if (resourceToFill.GetGUID() == GUID())
				continue;

			resource.key = resourceToFill.GetGUID();
			resource.projectPath = RelativeFilePath(currentFilePath);
			m_materialResources.Add(resource);
		}
	}
}

void Hail::ResourceRegistry::AddToRegistry(const FilePath& resourcePath, ResourceType type)
{
	MetaDataWithKey resource;
	resource.bIsLoaded = false;
	MetaResource resourceToFill;

	if (type == ResourceType::Texture)
	{
		TextureManager::LoadTextureMetaData(resourcePath, resourceToFill);

		if (resourceToFill.GetGUID() == GUID())
			return;

		bool isAlreadyLoaded = false;
		for (size_t i = 0; i < m_textureResources.Size(); i++)
		{
			if (m_textureResources[i].key == resourceToFill.GetGUID())
			{
				isAlreadyLoaded = true;
				break;
			}
		}
		if (isAlreadyLoaded)
			return;
	}
	if (type == ResourceType::Material)
	{
		MaterialManager::LoadMaterialMetaData(resourcePath, resourceToFill);
		if (resourceToFill.GetGUID() == GUID())
			return;
		bool isAlreadyLoaded = false;
		for (size_t i = 0; i < m_materialResources.Size(); i++)
		{
			if (m_materialResources[i].key == resourceToFill.GetGUID())
			{
				isAlreadyLoaded = true;
				break;
			}
		}
		if (isAlreadyLoaded)
			return;
	}

	resource.key = resourceToFill.GetGUID();
	resource.projectPath = RelativeFilePath(resourcePath);

	if (type == ResourceType::Texture)
		m_textureResources.Add(resource);
	if (type == ResourceType::Material)
		m_materialResources.Add(resource);
}

FilePath ResourceRegistry::GetProjectPath(ResourceType type, GUID resourceGuid) const
{
	if (type == ResourceType::Texture)
		return GetFilePathInternal(m_textureResources, resourceGuid);
	if (type == ResourceType::Material)
		return GetFilePathInternal(m_materialResources, resourceGuid);
	return FilePath();
}

bool Hail::ResourceRegistry::GetIsResourceImported(ResourceType type, GUID resourceGuid) const
{
	if (type == ResourceType::Texture)
		return GetIsResourceImportedInternal(m_textureResources, resourceGuid);
	if (type == ResourceType::Material)
		return GetIsResourceImportedInternal(m_materialResources, resourceGuid);
	return false;
}

bool Hail::ResourceRegistry::GetIsResourceLoaded(ResourceType type, GUID resourceGuid) const
{
	if (type == ResourceType::Texture)
		return GetIsResourceLoadedInternal(m_textureResources, resourceGuid);
	if (type == ResourceType::Material)
		return GetIsResourceLoadedInternal(m_materialResources, resourceGuid);
	return false;
}

void Hail::ResourceRegistry::SetResourceLoaded(ResourceType type, GUID resourceGuid)
{
	if (type == ResourceType::Texture)
		SetIsResourceLoadedInternal(m_textureResources, resourceGuid);
	if (type == ResourceType::Material)
		SetIsResourceLoadedInternal(m_materialResources, resourceGuid);

}

void Hail::ResourceRegistry::SetResourceUnloaded(ResourceType type, GUID resourceGuid)
{
	if (type == ResourceType::Texture)
		SetIsResourceUnloadedInternal(m_textureResources, resourceGuid);
	if (type == ResourceType::Material)
		SetIsResourceUnloadedInternal(m_materialResources, resourceGuid);
}

FilePath Hail::ResourceRegistry::GetFilePathInternal(const GrowingArray<MetaDataWithKey>& list, const GUID& resourceGuid) const
{
	for (size_t i = 0; i < list.Size(); i++)
		if (list[i].key == resourceGuid)
			return list[i].projectPath.GetFilePath();
	return FilePath();
}

bool Hail::ResourceRegistry::GetIsResourceImportedInternal(const GrowingArray<MetaDataWithKey>& list, const GUID& resourceGuid) const
{
	for (size_t i = 0; i < list.Size(); i++)
		if (list[i].key == resourceGuid)
			return true;
	return false;
}

bool Hail::ResourceRegistry::GetIsResourceLoadedInternal(const GrowingArray<MetaDataWithKey>& list, const GUID& resourceGuid) const
{
	for (size_t i = 0; i < list.Size(); i++)
		if (list[i].key == resourceGuid)
			return list[i].bIsLoaded;
	return false;
}

void Hail::ResourceRegistry::SetIsResourceLoadedInternal(GrowingArray<MetaDataWithKey>& list, const GUID& resourceGuid)
{
	for (size_t i = 0; i < m_textureResources.Size(); i++)
	{
		if (list[i].key == resourceGuid)
		{
			list[i].bIsLoaded = true;
			return;
		}
	}
}

void Hail::ResourceRegistry::SetIsResourceUnloadedInternal(GrowingArray<MetaDataWithKey>& list, const GUID& resourceGuid)
{
	for (size_t i = 0; i < m_textureResources.Size(); i++)
	{
		if (list[i].key == resourceGuid)
		{
			list[i].bIsLoaded = false;
			return;
		}
	}
}
