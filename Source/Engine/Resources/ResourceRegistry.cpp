#include "Engine_PCH.h"

#include "ResourceRegistry.h"
#include "Utility\FileSystem.h"
#include "TextureManager.h"
using namespace Hail;

void ResourceRegistry::Init()
{
	m_textureResources.Init(16);
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
	}
}

void Hail::ResourceRegistry::AddToRegistry(const FilePath& projectPath, ResourceType type)
{
	MetaDataWithKey resource;
	resource.bIsLoaded = false;
	//TODO: Add support for other resource types

	MetaResource resourceToFill;
	TextureManager::LoadTextureMetaData(projectPath, resourceToFill);

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
	resource.key = resourceToFill.GetGUID();
	resource.projectPath = RelativeFilePath(projectPath);
	m_textureResources.Add(resource);
}

FilePath ResourceRegistry::GetProjectPath(ResourceType type, GUID resourceGuid) const
{
	//TODO: Add support for other resource types
	for (size_t i = 0; i < m_textureResources.Size(); i++)
	{
		if (m_textureResources[i].key == resourceGuid)
			return m_textureResources[i].projectPath.GetFilePath();
	}
	return FilePath();
}

bool Hail::ResourceRegistry::GetIsResourceImported(ResourceType type, GUID resourceGuid) const
{
	//TODO: Add support for other resource types
	for (size_t i = 0; i < m_textureResources.Size(); i++)
	{
		if (m_textureResources[i].key == resourceGuid)
			return true;
	}
	return false;
}

bool Hail::ResourceRegistry::GetIsResourceLoaded(ResourceType type, GUID resourceGuid) const
{
	//TODO: Add support for other resource types
	for (size_t i = 0; i < m_textureResources.Size(); i++)
	{
		if (m_textureResources[i].key == resourceGuid)
			return m_textureResources[i].bIsLoaded;
	}
	return false;
}

void Hail::ResourceRegistry::SetResourceLoaded(ResourceType type, GUID resourceGuid)
{
	//TODO: Add support for other resource types
	for (size_t i = 0; i < m_textureResources.Size(); i++)
	{
		if (m_textureResources[i].key == resourceGuid)
		{
			m_textureResources[i].bIsLoaded = true;
			return;
		}
	}
}

void Hail::ResourceRegistry::SetResourceUnloaded(ResourceType type, GUID resourceGuid)
{
	//TODO: Add support for other resource types
	for (size_t i = 0; i < m_textureResources.Size(); i++)
	{
		if (m_textureResources[i].key == resourceGuid)
		{
			m_textureResources[i].bIsLoaded = false;
			return;
		}

	}
}
