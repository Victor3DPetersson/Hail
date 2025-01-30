#include "Engine_PCH.h"

#include "ResourceRegistry.h"
#include "Utility\FileSystem.h"
#include "MaterialManager.h"
#include "TextureManager.h"
using namespace Hail;

void ResourceRegistry::Init()
{
	RecursiveFileIterator fileIterator(FilePath::GetCurrentWorkingDirectory());

	//TODO: Check if current path does not match the meta resource project path, and make a function to clean that up.

	while (fileIterator.IterateOverFolderRecursively())
	{
		FilePath currentFilePath = fileIterator.GetCurrentPath();
		const FileObject& currentFileObject = currentFilePath.Object();

		MetaData resource;
		resource.m_state = eResourceState::Unloaded;

		if (StringCompare(currentFileObject.Extension(), L"txr"))
		{
			MetaResource resourceToFill;
			TextureManager::LoadTextureMetaData(currentFilePath, resourceToFill);

			if (resourceToFill.GetGUID() == GUID())
				continue;

			resource.m_resource = resourceToFill;
			m_textureResources.Add(resource);
		}
		if (StringCompare(currentFileObject.Extension(), L"mat"))
		{
			MetaResource resourceToFill;
			MaterialManager::LoadMaterialMetaData(currentFilePath, resourceToFill);
			if (resourceToFill.GetGUID() == GUID())
				continue;

			resource.m_resource = resourceToFill;
			m_materialResources.Add(resource);
		}
		if (StringCompare(currentFileObject.Extension(), L"shr"))
		{
			MetaResource resourceToFill = MaterialManager::LoadShaderMetaData(currentFilePath);
			if (resourceToFill.GetGUID() == GUID())
				continue;
			resource.m_resource = resourceToFill;
			m_shaderResources.Add(resource);
		}
	}
}

void Hail::ResourceRegistry::AddToRegistry(const FilePath& resourcePath, ResourceType type)
{
	MetaData resource;
	resource.m_state = eResourceState::Unloaded;
	MetaResource resourceToFill;

	if (type == ResourceType::Texture)
	{
		TextureManager::LoadTextureMetaData(resourcePath, resourceToFill);

		if (resourceToFill.GetGUID() == GUID())
			return;

		bool isAlreadyLoaded = false;
		for (size_t i = 0; i < m_textureResources.Size(); i++)
		{
			if (m_textureResources[i].m_resource.GetGUID() == resourceToFill.GetGUID())
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
			if (m_materialResources[i].m_resource.GetGUID() == resourceToFill.GetGUID())
			{
				isAlreadyLoaded = true;
				break;
			}
		}
		if (isAlreadyLoaded)
			return;
	}
	if (type == ResourceType::Shader)
	{
		resourceToFill = MaterialManager::LoadShaderMetaData(resourcePath);
		if (resourceToFill.GetGUID() == GUID())
			return;
		bool isAlreadyLoaded = false;
		for (size_t i = 0; i < m_shaderResources.Size(); i++)
		{
			if (m_shaderResources[i].m_resource.GetGUID() == resourceToFill.GetGUID())
			{
				isAlreadyLoaded = true;
				break;
			}
		}
		if (isAlreadyLoaded)
			return;
	}

	resource.m_resource = resourceToFill;

	if (type == ResourceType::Texture)
		m_textureResources.Add(resource);
	if (type == ResourceType::Material)
		m_materialResources.Add(resource);
	if (type == ResourceType::Shader)
		m_shaderResources.Add(resource);
}

FilePath ResourceRegistry::GetProjectPath(ResourceType type, GUID resourceGuid) const
{
	if (type == ResourceType::Texture)
		return GetProjectFilePathInternal(m_textureResources, resourceGuid);
	if (type == ResourceType::Material)
		return GetProjectFilePathInternal(m_materialResources, resourceGuid);
	if (type == ResourceType::Shader)
		return GetProjectFilePathInternal(m_shaderResources, resourceGuid);
	return FilePath();
}

FilePath Hail::ResourceRegistry::GetSourcePath(ResourceType type, GUID resourceGuid) const
{
	if (type == ResourceType::Texture)
		return GetSourceFilePathInternal(m_textureResources, resourceGuid);
	if (type == ResourceType::Material)
		return GetSourceFilePathInternal(m_materialResources, resourceGuid);
	if (type == ResourceType::Shader)
		return GetSourceFilePathInternal(m_shaderResources, resourceGuid);
	return FilePath();
}

bool Hail::ResourceRegistry::GetIsResourceImported(ResourceType type, GUID resourceGuid) const
{
	if (type == ResourceType::Texture)
		return GetIsResourceImportedInternal(m_textureResources, resourceGuid);
	if (type == ResourceType::Material)
		return GetIsResourceImportedInternal(m_materialResources, resourceGuid);
	if (type == ResourceType::Shader)
		return GetIsResourceImportedInternal(m_shaderResources, resourceGuid);
	return false;
}

bool Hail::ResourceRegistry::GetIsResourceLoaded(ResourceType type, GUID resourceGuid) const
{
	if (type == ResourceType::Texture)
		return GetIsResourceLoadedInternal(m_textureResources, resourceGuid);
	if (type == ResourceType::Material)
		return GetIsResourceLoadedInternal(m_materialResources, resourceGuid);
	if (type == ResourceType::Shader)
		return GetIsResourceLoadedInternal(m_shaderResources, resourceGuid);
	return false;
}

eResourceState Hail::ResourceRegistry::GetResourceState(ResourceType type, GUID resourceGuid) const
{
	if (type == ResourceType::Texture)
		return GetResourceStateInternal(m_textureResources, resourceGuid);
	if (type == ResourceType::Material)
		return GetResourceStateInternal(m_materialResources, resourceGuid);
	if (type == ResourceType::Shader)
		return GetResourceStateInternal(m_shaderResources, resourceGuid);
	return eResourceState::Invalid;
}

void Hail::ResourceRegistry::SetResourceLoaded(ResourceType type, GUID resourceGuid)
{
	if (type == ResourceType::Texture)
		SetIsResourceLoadedInternal(m_textureResources, resourceGuid);
	if (type == ResourceType::Material)
		SetIsResourceLoadedInternal(m_materialResources, resourceGuid);
	if (type == ResourceType::Shader)
		SetIsResourceLoadedInternal(m_shaderResources, resourceGuid);
}

void Hail::ResourceRegistry::SetResourceUnloaded(ResourceType type, GUID resourceGuid)
{
	if (type == ResourceType::Texture)
		SetIsResourceUnloadedInternal(m_textureResources, resourceGuid);
	if (type == ResourceType::Material)
		SetIsResourceUnloadedInternal(m_materialResources, resourceGuid);
	if (type == ResourceType::Shader)
		SetIsResourceUnloadedInternal(m_shaderResources, resourceGuid);
}

void Hail::ResourceRegistry::SetResourceLoadFailed(ResourceType type, GUID resourceGuid)
{
	if (type == ResourceType::Texture)
		SetIsResourceLoadFailedInternal(m_textureResources, resourceGuid);
	if (type == ResourceType::Material)
		SetIsResourceLoadFailedInternal(m_materialResources, resourceGuid);
	if (type == ResourceType::Shader)
		SetIsResourceLoadFailedInternal(m_shaderResources, resourceGuid);
}

String64 Hail::ResourceRegistry::GetResourceName(ResourceType type, GUID resourceGUID) const
{
	if (type == ResourceType::Texture)
		return	GetResourceNameInternal(m_textureResources, resourceGUID);
	if (type == ResourceType::Material)
		return	GetResourceNameInternal(m_materialResources, resourceGUID);
	if (type == ResourceType::Shader)
		return	GetResourceNameInternal(m_shaderResources, resourceGUID);

	return String64();
}

bool Hail::ResourceRegistry::IsResourceOutOfDate(ResourceType type, GUID resourceGUID)
{
	if (type == ResourceType::Texture)
	{
		return IsResourceOutOfDateInternal(m_textureResources, resourceGUID);
	}
	if (type == ResourceType::Material)
	{
		H_ASSERT(false, "Do not try to check if a material is out of date, as it is not depending on a source file.");
		return false;
	}
	if (type == ResourceType::Shader)
	{
		return IsResourceOutOfDateInternal(m_shaderResources, resourceGUID);
	}
	return false;
}

const MetaResource* Hail::ResourceRegistry::GetResourceMetaInformation(ResourceType type, const FilePath& pathToCheck) const
{
	if (type == ResourceType::Texture)
	{
		return GetMetaResourceInternal(m_textureResources, pathToCheck);
	}
	if (type == ResourceType::Material)
	{
		return GetMetaResourceInternal(m_materialResources, pathToCheck);
	}
	if (type == ResourceType::Shader)
	{
		return GetMetaResourceInternal(m_shaderResources, pathToCheck);
	}
	return nullptr;
}

FilePath Hail::ResourceRegistry::GetProjectFilePathInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const
{
	for (size_t i = 0; i < list.Size(); i++)
		if (list[i].m_resource.GetGUID() == resourceGuid)
			return list[i].m_resource.GetProjectFilePath().GetFilePath();
	return FilePath();
}

FilePath Hail::ResourceRegistry::GetSourceFilePathInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const
{
	for (size_t i = 0; i < list.Size(); i++)
		if (list[i].m_resource.GetGUID() == resourceGuid)
			return list[i].m_resource.GetSourceFilePath().GetFilePath();
	return FilePath();
}

bool Hail::ResourceRegistry::GetIsResourceImportedInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const
{
	for (size_t i = 0; i < list.Size(); i++)
		if (list[i].m_resource.GetGUID() == resourceGuid)
			return true;
	return false;
}

bool Hail::ResourceRegistry::GetIsResourceLoadedInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const
{
	for (size_t i = 0; i < list.Size(); i++)
		if (list[i].m_resource.GetGUID() == resourceGuid)
			return list[i].m_state == eResourceState::Loaded;
	return false;
}

bool Hail::ResourceRegistry::IsResourceOutOfDateInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const
{
	for (size_t i = 0; i < list.Size(); i++)
	{
		if (list[i].m_resource.GetGUID() == resourceGuid)
		{
			CommonFileData sourceCurrentFileData = list[i].m_resource.GetSourceFilePath().GetFilePath().Object().GetFileData();
			const CommonFileData& serializedSourceFileData = list[i].m_resource.GetSourceFileData();
			return sourceCurrentFileData.m_lastWriteTime.m_highDateTime != serializedSourceFileData.m_lastWriteTime.m_highDateTime || 
				sourceCurrentFileData.m_lastWriteTime.m_lowDateTime != serializedSourceFileData.m_lastWriteTime.m_lowDateTime;
		}
	}
	return false;
}

void Hail::ResourceRegistry::SetIsResourceLoadedInternal(GrowingArray<MetaData>& list, const GUID& resourceGuid)
{
	for (size_t i = 0; i < m_textureResources.Size(); i++)
	{
		if (list[i].m_resource.GetGUID() == resourceGuid)
		{
			list[i].m_state = eResourceState::Loaded;
			return;
		}
	}
}

void Hail::ResourceRegistry::SetIsResourceUnloadedInternal(GrowingArray<MetaData>& list, const GUID& resourceGuid)
{
	for (size_t i = 0; i < m_textureResources.Size(); i++)
	{
		if (list[i].m_resource.GetGUID() == resourceGuid)
		{
			list[i].m_state = eResourceState::Unloaded;
			return;
		}
	}
}

void Hail::ResourceRegistry::SetIsResourceLoadFailedInternal(GrowingArray<MetaData>& list, const GUID& resourceGuid)
{
	for (size_t i = 0; i < m_textureResources.Size(); i++)
	{
		if (list[i].m_resource.GetGUID() == resourceGuid)
		{
			list[i].m_state = eResourceState::Invalid;
			return;
		}
	}
}

String64 Hail::ResourceRegistry::GetResourceNameInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const
{
	for (size_t i = 0; i < list.Size(); i++)
		if (list[i].m_resource.GetGUID() == resourceGuid)
			return list[i].m_resource.GetName();
	return false;
}

const Hail::MetaResource* Hail::ResourceRegistry::GetMetaResourceInternal(const GrowingArray<MetaData>& list, const FilePath& pathToCheck) const
{
	for (size_t i = 0; i < list.Size(); i++)
	{
		const MetaData& meta = list[i];
		if (meta.m_resource.GetProjectFilePath().GetFilePath() == pathToCheck)
			return &meta.m_resource;
	}

	return nullptr;
}

eResourceState Hail::ResourceRegistry::GetResourceStateInternal(const GrowingArray<MetaData>& list, const GUID& resourceGuid) const
{
	for (size_t i = 0; i < list.Size(); i++)
	{
		for (size_t i = 0; i < list.Size(); i++)
			if (list[i].m_resource.GetGUID() == resourceGuid)
				return list[i].m_state;
	}
	return eResourceState::Invalid;
}
