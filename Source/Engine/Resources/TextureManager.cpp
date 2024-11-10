#include "Engine_PCH.h"
#include "TextureManager.h"

#include "DebugMacros.h"
#include "TextureCompiler.h"

#include "Utility\FileSystem.h"
#include "Utility\StringUtility.h"
#include "Utility\InOutStream.h"

#include "MetaResource.h"
#include "HailEngine.h"
#include "ResourceRegistry.h"

namespace
{

	//TODO: Move the memory that is used here to a temporary memory buffer
	void Read8BitStream(Hail::InOutStream& stream, void** outData, const uint32_t numberOfBytesToRead)
	{
		*outData = new uint8_t[numberOfBytesToRead];
		unsigned char* pixels = (unsigned char*)malloc(numberOfBytesToRead);
		stream.Read((char*)&pixels[0], numberOfBytesToRead);
		memcpy(*outData, pixels, numberOfBytesToRead);
		free(pixels);
	}
	void Read16BitStream(Hail::InOutStream& stream, void** outData, const uint32_t numberOfBytesToRead)
	{
		*outData = new uint16_t[numberOfBytesToRead];
		uint16_t* pixels = (uint16_t*)malloc(numberOfBytesToRead);
		stream.Read((char*)&pixels[0], numberOfBytesToRead);
		memcpy(*outData, pixels, numberOfBytesToRead);
		free(pixels);
	}
	void Read32UintBitStream(Hail::InOutStream& stream, void** outData, const uint32_t numberOfBytesToRead)
	{
		*outData = new uint32_t[numberOfBytesToRead];
		uint32_t* pixels = (uint32_t*)malloc(numberOfBytesToRead);
		stream.Read((char*)&pixels[0], numberOfBytesToRead);
		memcpy(*outData, pixels, numberOfBytesToRead);
		free(pixels);
	}
	void Read32FltBitStream(Hail::InOutStream& stream, void** outData, const uint32_t numberOfBytesToRead)
	{
		*outData = new float[numberOfBytesToRead];
		float* pixels = (float*)malloc(numberOfBytesToRead);
		stream.Read((char*)&pixels[0], numberOfBytesToRead);
		memcpy(*outData, pixels, numberOfBytesToRead);
		free(pixels);
	}
}

using namespace Hail;

void TextureManager::Init(RenderingDevice* device)
{
	m_device = device;
	CreateDefaultTexture();
}

void Hail::TextureManager::ClearAllResources()
{
	m_defaultTexture->CleanupResource(m_device);
	for (size_t i = 0; i < m_loadedTextures.Size(); i++)
	{
		m_loadedTextures[i]->CleanupResource(m_device);
	}
	m_loadedTextures.RemoveAll();
}

void Hail::TextureManager::ReloadAllTextures(uint32 frameInFlight)
{
	for (uint32 i = 0; i < m_loadedTextures.Size(); i++)
	{
		if (!ReloadTextureInternal(i, frameInFlight))
		{
			//TODO: Add an assert here to catch this error
		}
	}
}

void TextureManager::Update()
{
	//TODO: Add file watcher here
}

bool Hail::TextureManager::LoadTexture(const char* textureName)
{
	MetaResource metaData;
	//TODO: hook up meta data to engine handling of resource'
	CompiledTexture compiledTextureData = LoadTextureInternal(textureName, metaData, false);
	if (compiledTextureData.loadState == TEXTURE_LOADSTATE::LOADED_TO_RAM)
	{
		TextureResource* textureResource = CreateTextureInternal(textureName, compiledTextureData);
		textureResource->m_index = TextureResource::g_idCounter++;
		textureResource->m_metaResource = metaData;
		m_loadedTextures.Add(textureResource);
		if (textureResource)
			GetResourceRegistry().SetResourceLoaded(ResourceType::Texture, metaData.GetGUID());
		else
			GetResourceRegistry().SetResourceUnloaded(ResourceType::Texture, metaData.GetGUID());

		return textureResource;
	}
	return false;
}

uint32 Hail::TextureManager::LoadTexture(GUID textureID)
{
	if (GetResourceRegistry().GetIsResourceLoaded(ResourceType::Texture, textureID))
	{
		for (uint32 i = 0; i < m_loadedTextures.Size(); i++)
		{
			TextureResource* pTexture = m_loadedTextures[i];
			if (pTexture->m_metaResource.GetGUID() == textureID)
				return i;
		}
	}

	TextureResource* pTexture = LoadTextureInternalPath(GetResourceRegistry().GetProjectPath(ResourceType::Texture, textureID));
	if (pTexture)
	{
		pTexture->m_index = TextureResource::g_idCounter++;
		m_loadedTextures.Add(pTexture);
		GetResourceRegistry().SetResourceLoaded(ResourceType::Texture, pTexture->m_metaResource.GetGUID());
		return pTexture->m_index;
	}
	GetResourceRegistry().SetResourceUnloaded(ResourceType::Texture, textureID);
	return INVALID_TEXTURE_HANDLE;
}


void Hail::TextureManager::ClearTextureInternalForReload(int textureIndex, uint32 frameInFlight)
{
	m_loadedTextures[textureIndex]->m_validator.MarkResourceAsDirty(frameInFlight);
	if (m_loadedTextures[textureIndex]->m_compiledTextureData.loadState == TEXTURE_LOADSTATE::LOADED_TO_RAM)
	{
		DeleteCompiledTexture(m_loadedTextures[textureIndex]->m_compiledTextureData);
	}
}

bool Hail::TextureManager::ReloadTextureInternal(int textureIndex, uint32 frameInFlight)
{
	ClearTextureInternalForReload(textureIndex, frameInFlight);
	if (m_loadedTextures[textureIndex]->m_validator.IsAllFrameResourcesDirty())
	{
		//TODO: hook up meta data to engine handling of resource
		m_loadedTextures[textureIndex]->m_compiledTextureData = LoadTextureInternal(m_loadedTextures[textureIndex]->textureName, m_loadedTextures[textureIndex]->m_metaResource, true);
		if (m_loadedTextures[textureIndex]->m_compiledTextureData.loadState != TEXTURE_LOADSTATE::LOADED_TO_RAM)
		{
			return false;
		}
	}
	m_loadedTextures[textureIndex]->m_validator.ClearFrameData(frameInFlight);
	return true;
}

CompiledTexture TextureManager::LoadTextureInternal(const char* textureName, MetaResource& metaResourceToFill, bool reloadTexture)
{
	CompiledTexture returnTexture;
	StringL inPath = StringL::Format("%s%s%s", TEXTURES_DIR_OUT, textureName, ".txr");

	InOutStream inStream;

	if (!inStream.OpenFile(inPath.Data(), FILE_OPEN_TYPE::READ, true) || reloadTexture)
	{
		if (!CompileTexture(textureName))
		{
			return returnTexture;
		}
	}
	inStream.OpenFile(inPath.Data(), FILE_OPEN_TYPE::READ, true);

	if (!ReadStreamInternal(returnTexture, inStream, metaResourceToFill))
		return returnTexture;

	inStream.CloseFile();

	returnTexture.loadState = TEXTURE_LOADSTATE::LOADED_TO_RAM;
	return returnTexture;
}

TextureResource* Hail::TextureManager::LoadTextureInternalPath(const FilePath& path)
{
	InOutStream inStream;

	MetaResource metaData;
	CompiledTexture compiledTextureData;
	if (!inStream.OpenFile(path, FILE_OPEN_TYPE::READ, true))
		return false;

	if (!ReadStreamInternal(compiledTextureData, inStream, metaData))
		return false;

	inStream.CloseFile();
	compiledTextureData.loadState = TEXTURE_LOADSTATE::LOADED_TO_RAM;

	TextureResource* textureResource = nullptr;
	if (compiledTextureData.loadState == TEXTURE_LOADSTATE::LOADED_TO_RAM)
	{
		textureResource = CreateTextureInternal(path.Object().Name().CharString(), compiledTextureData);
		textureResource->m_metaResource = metaData;
	}
	return textureResource;
}

bool Hail::TextureManager::ReadStreamInternal(CompiledTexture& textureToFill, InOutStream& inStream, MetaResource& metaResourceToFill) const
{
	inStream.Read((char*)&textureToFill.header, sizeof(TextureHeader));
	switch (ToEnum<TEXTURE_TYPE>(textureToFill.header.textureType))
	{
	case TEXTURE_TYPE::R8G8B8A8_SRGB:
		Read8BitStream(inStream, &textureToFill.compiledColorValues, GetTextureByteSize(textureToFill.header));
		break;
	case TEXTURE_TYPE::R8G8B8_SRGB:
	{
		void* tempData = nullptr;
		Read8BitStream(inStream, &tempData, GetTextureByteSize(textureToFill.header));
		textureToFill.header.textureType = static_cast<uint32_t>(TEXTURE_TYPE::R8G8B8A8_SRGB);
		textureToFill.compiledColorValues = new uint8_t[1 * 4 * textureToFill.header.width * textureToFill.header.height];
		uint32_t rgbIterator = 0;
		for (size_t i = 0; i < 4 * textureToFill.header.width * textureToFill.header.height; i++)
		{
			switch (i % 4)
			{
			case 0:
				((uint8_t*)textureToFill.compiledColorValues)[i] = static_cast<uint8_t*>(tempData)[rgbIterator++];
				break;
			case 1:
				((uint8_t*)textureToFill.compiledColorValues)[i] = static_cast<uint8_t*>(tempData)[rgbIterator++];
				break;
			case 2:
				((uint8_t*)textureToFill.compiledColorValues)[i] = static_cast<uint8_t*>(tempData)[rgbIterator++];
				break;
			case 3:
				((uint8_t*)textureToFill.compiledColorValues)[i] = 255;
				break;
			}
		}
		delete[] tempData;
	}
	break;
	case TEXTURE_TYPE::R8G8B8A8:
		Read8BitStream(inStream, &textureToFill.compiledColorValues, GetTextureByteSize(textureToFill.header));
		break;
	case TEXTURE_TYPE::R8G8B8:
	{
		void* tempData = nullptr;
		Read8BitStream(inStream, &tempData, GetTextureByteSize(textureToFill.header));
		textureToFill.header.textureType = static_cast<uint32_t>(TEXTURE_TYPE::R8G8B8A8);
		textureToFill.compiledColorValues = new uint8_t[1 * 4 * textureToFill.header.width * textureToFill.header.height];
		uint32_t rgbIterator = 0;
		for (size_t i = 0; i < 4 * textureToFill.header.width * textureToFill.header.height; i++)
		{
			switch (i % 4)
			{
			case 0:
				((uint8_t*)textureToFill.compiledColorValues)[i] = static_cast<uint8_t*>(tempData)[rgbIterator++];
				break;
			case 1:
				((uint8_t*)textureToFill.compiledColorValues)[i] = static_cast<uint8_t*>(tempData)[rgbIterator++];
				break;
			case 2:
				((uint8_t*)textureToFill.compiledColorValues)[i] = static_cast<uint8_t*>(tempData)[rgbIterator++];
				break;
			case 3:
				((uint8_t*)textureToFill.compiledColorValues)[i] = 255;
				break;
			}
		}
		delete[] tempData;
	}
	break;
	case TEXTURE_TYPE::R16G16B16A16:
		Read16BitStream(inStream, &textureToFill.compiledColorValues, GetTextureByteSize(textureToFill.header));
		break;
	case TEXTURE_TYPE::R16G16B16:
		Read16BitStream(inStream, &textureToFill.compiledColorValues, GetTextureByteSize(textureToFill.header));
		break;
	case TEXTURE_TYPE::R16:
		Read16BitStream(inStream, &textureToFill.compiledColorValues, GetTextureByteSize(textureToFill.header));
		break;
	case TEXTURE_TYPE::R32G32B32A32:
		Read32UintBitStream(inStream, &textureToFill.compiledColorValues, GetTextureByteSize(textureToFill.header));
		break;
	case TEXTURE_TYPE::R32G32B32:
		Read32UintBitStream(inStream, &textureToFill.compiledColorValues, GetTextureByteSize(textureToFill.header));
		break;
	case TEXTURE_TYPE::R32:
		Read32UintBitStream(inStream, &textureToFill.compiledColorValues, GetTextureByteSize(textureToFill.header));
		break;
	default:
		return false;
		break;
	}
	const uint64 sizeLeft = inStream.GetFileSize() - inStream.GetFileSeekPosition();
	if (sizeLeft != 0)
	{
		metaResourceToFill.Deserialize(inStream);
	}
	return true;
}

bool TextureManager::CompileTexture(const char* textureName)
{
	RecursiveFileIterator fileIterator = RecursiveFileIterator(TEXTURES_DIR_IN);
	bool foundTexture = false;

	FilePath currentPath;
	String64 filenameStr;
	while (fileIterator.IterateOverFolderRecursively())
	{
		currentPath = fileIterator.GetCurrentPath();
		if (currentPath.IsFile())
		{
			currentPath = fileIterator.GetCurrentPath();
			const FileObject& currentFileObject = currentPath.Object();
			FromWCharToConstChar(currentFileObject.Name(), filenameStr, 64);
			if (StringCompare(filenameStr, textureName) 
				&& StringCompare(L"tga", currentFileObject.Extension()) 
				|| StringCompare(L"TGA", currentFileObject.Extension()))
			{
				foundTexture = true;
				break;
			}
		}
	}

	if (!foundTexture)
	{
		Debug_PrintConsoleConstChar(StringL::Format("Could not find texture : %s", textureName));
		return false;
	}
	FilePath projectPath = TextureCompiler::CompileSpecificTGATexture(currentPath);
	if (TextureCompiler::CompileSpecificTGATexture(currentPath).IsValid())
	{
		GetResourceRegistry().AddToRegistry(projectPath, ResourceType::Texture);
		return true;
	}
	return false;
}

TextureResource* Hail::TextureManager::GetTexture(uint32_t index)
{
	for (size_t i = 0; i < m_loadedTextures.Size(); i++)
	{
		if (m_loadedTextures[i]->m_index == index)
			return m_loadedTextures[i];
	}

	return nullptr;
}

TextureResource* Hail::TextureManager::GetEngineTexture(eDecorationSets setDomainToGet, uint32_t bindingIndex, uint32 frameInFlight)
{
	// TODO: Assert on setDomain
	if (setDomainToGet == eDecorationSets::GlobalDomain)
	{

	}
	else if (setDomainToGet == eDecorationSets::MaterialTypeDomain)
	{
		return m_materialTextures[bindingIndex][frameInFlight];
	}
	return nullptr;
}

void Hail::TextureManager::RegisterEngineTexture(TextureResource* textureToSet, eDecorationSets setDomain, uint32 bindingIndex, uint32 frameInFlight)
{
	// TODO: Assert on setDomain
	if (setDomain == eDecorationSets::GlobalDomain)
	{

	}
	else if (setDomain == eDecorationSets::MaterialTypeDomain)
	{
		m_materialTextures[bindingIndex][frameInFlight] = textureToSet;
	}
}

FilePath Hail::TextureManager::ImportTextureResource(const FilePath& filepath) const
{
	FilePath projectPath = TextureCompiler::CompileSpecificTGATexture(filepath);
	GetResourceRegistry().AddToRegistry(projectPath, ResourceType::Texture);
	return projectPath;
}

void Hail::TextureManager::LoadTextureMetaData(const FilePath& filePath, MetaResource& metaResourceToFill)
{
	if (!StringCompare(filePath.Object().Extension(), L"txr"))
		return;

	InOutStream inStream;
	if (!inStream.OpenFile(filePath.Data(), FILE_OPEN_TYPE::READ, true))
		return;

	CompiledTexture textureToFill;
	inStream.Read((char*)&textureToFill.header, sizeof(TextureHeader));
	inStream.Seek(GetTextureByteSize(textureToFill.header), 1);

	const uint64 sizeLeft = inStream.GetFileSize() - inStream.GetFileSeekPosition();
	if (sizeLeft != 0)
	{
		metaResourceToFill.Deserialize(inStream);
	}
}

void Hail::TextureManager::CreateDefaultTexture()
{
	const uint8 widthHeight = 16;

	CompiledTexture compiledTextureData;
	compiledTextureData.header.height = widthHeight;
	compiledTextureData.header.width = widthHeight;
	compiledTextureData.header.textureType = (uint32)TEXTURE_TYPE::R8G8B8A8_SRGB;
	compiledTextureData.compiledColorValues = new uint8[GetTextureByteSize(compiledTextureData.header)];
	compiledTextureData.loadState = TEXTURE_LOADSTATE::LOADED_TO_RAM;
	struct Color
	{
		uint8 r = 255;
		uint8 g = 255;
		uint8 b = 255;
		uint8 a = 255;
	};

	Color color1;
	color1.r = 89;
	color1.g = 104;
	color1.b = 128;

	Color color2;
	color2.r = 41;
	color2.g = 51;
	color2.b = 42;

	//multiply with 4, so we cover each color
	for (uint8 x = 0; x < widthHeight * 4; x = x + 4)
	{
		const uint8 xIndexValue = (x / 4) % 4;
		for (uint8 y = 0; y < widthHeight * 4; y = y + 4)
		{
			const uint8 yIndexValue = ((y * widthHeight) / 4) % 64;
			if (xIndexValue == 0 || xIndexValue == 1)
			{
				if ((yIndexValue == 0 || yIndexValue == 16))
					memcpy(&((uint8*)compiledTextureData.compiledColorValues)[x + y * widthHeight], &color1, sizeof(Color));
				else
					memcpy(&((uint8*)compiledTextureData.compiledColorValues)[x + y * widthHeight], &color2, sizeof(Color));
			}
			else
			{
				if ((yIndexValue == 32 || yIndexValue == 48))
					memcpy(&((uint8*)compiledTextureData.compiledColorValues)[x + y * widthHeight], &color1, sizeof(Color));
				else
					memcpy(&((uint8*)compiledTextureData.compiledColorValues)[x + y * widthHeight], &color2, sizeof(Color));
			}

		}
	}
	m_defaultTexture = CreateTextureInternal("Default Texture", compiledTextureData);
	m_defaultTexture->m_index = MAX_UINT;
}