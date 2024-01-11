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
	const char* REQUIRED_TEXTURES[REQUIRED_TEXTURE_COUNT] =
	{
		"cloud shapes test",
		"Debug_Grid",
		"spaceShip"
	};
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
	m_textureCommonData.Init(10);
	m_textureCommonDataValidators.Init(10);
	CreateDefaultTexture();
}

void Hail::TextureManager::ReloadAllTextures(uint32 frameInFlight)
{
	for (uint32 i = 0; i < m_textureCommonData.Size(); i++)
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
	TextureResource textureResource;
	MetaResource metaData;
	//TODO: hook up meta data to engine handling of resource
	if (LoadTextureInternal(textureName, textureResource, metaData, false))
	{
		textureResource.index = m_textureCommonData.Size();
		m_textureCommonData.Add(textureResource);
		const bool creationResult = CreateTextureInternal(m_textureCommonData.GetLast(), false);
		if (creationResult)
			GetResourceRegistry().SetResourceLoaded(ResourceType::Texture, metaData.GetGUID());
		else
			GetResourceRegistry().SetResourceUnloaded(ResourceType::Texture, metaData.GetGUID());

		return creationResult;
	}
	return false;
}

bool TextureManager::LoadAllRequiredTextures()
{
	for (uint32 i = 0; i < REQUIRED_TEXTURE_COUNT; i++)
	{
		if (!LoadTexture(REQUIRED_TEXTURES[i]))
		{
			return false;
		}
		m_textureCommonDataValidators.Add(ResourceValidator());
	}
	return true;
}

void Hail::TextureManager::ClearTextureInternalForReload(int textureIndex, uint32 frameInFlight)
{
	m_textureCommonDataValidators[textureIndex].MarkResourceAsDirty(frameInFlight);
	if (m_textureCommonData[textureIndex].m_compiledTextureData.loadState == TEXTURE_LOADSTATE::LOADED_TO_RAM)
	{
		DeleteCompiledTexture(m_textureCommonData[textureIndex].m_compiledTextureData);
	}
}

bool Hail::TextureManager::ReloadTextureInternal(int textureIndex, uint32 frameInFlight)
{
	ClearTextureInternalForReload(textureIndex, frameInFlight);
	if (m_textureCommonDataValidators[textureIndex].IsAllFrameResourcesDirty())
	{
		//TODO: hook up meta data to engine handling of resource
		MetaResource metaData;
		if (!LoadTextureInternal(m_textureCommonData[textureIndex].textureName, m_textureCommonData[textureIndex], metaData, true))
		{
			return false;
		}
	}
	m_textureCommonDataValidators[textureIndex].ClearFrameData(frameInFlight);
	return true;
}

bool TextureManager::LoadTextureInternal(const char* textureName, TextureResource& textureToFill, MetaResource& metaResourceToFill, bool reloadTexture)
{
	String256 inPath = String256::Format("%s%s%s", TEXTURES_DIR_OUT, textureName, ".txr");

	InOutStream inStream;

	if (!inStream.OpenFile(inPath.Data(), FILE_OPEN_TYPE::READ, true) || reloadTexture)
	{
		if (!CompileTexture(textureName))
		{
			return false;
		}
	}
	inStream.OpenFile(inPath.Data(), FILE_OPEN_TYPE::READ, true);

	//Debug_PrintConsoleString256(String256::Format("\nImporting Texture:\n%s:", textureName));

	inStream.Read((char*)&textureToFill.m_compiledTextureData.header, sizeof(TextureHeader));
	//Debug_PrintConsoleString256(String256::Format("Texture Width:%i Heigth:%i :%s", textureToFill.m_compiledTextureData.header.width, textureToFill.m_compiledTextureData.header.height, "\n"));
	switch (ToEnum<TEXTURE_TYPE>(textureToFill.m_compiledTextureData.header.textureType))
	{
	case TEXTURE_TYPE::R8G8B8A8_SRGB:
		Read8BitStream(inStream, &textureToFill.m_compiledTextureData.compiledColorValues, GetTextureByteSize(textureToFill.m_compiledTextureData.header));
		break;
	case TEXTURE_TYPE::R8G8B8_SRGB:
	{
		void* tempData = nullptr;
		Read8BitStream(inStream, &tempData, GetTextureByteSize(textureToFill.m_compiledTextureData.header));
		textureToFill.m_compiledTextureData.header.textureType = static_cast<uint32_t>(TEXTURE_TYPE::R8G8B8A8_SRGB);
		textureToFill.m_compiledTextureData.compiledColorValues = new uint8_t[1 * 4 * textureToFill.m_compiledTextureData.header.width * textureToFill.m_compiledTextureData.header.height];
		uint32_t rgbIterator = 0;
		for (size_t i = 0; i < 4 * textureToFill.m_compiledTextureData.header.width * textureToFill.m_compiledTextureData.header.height; i++)
		{
			switch (i % 4)
			{
			case 0:
				((uint8_t*)textureToFill.m_compiledTextureData.compiledColorValues)[i] = static_cast<uint8_t*>(tempData)[rgbIterator++];
				break;
			case 1:
				((uint8_t*)textureToFill.m_compiledTextureData.compiledColorValues)[i] = static_cast<uint8_t*>(tempData)[rgbIterator++];
				break;
			case 2:
				((uint8_t*)textureToFill.m_compiledTextureData.compiledColorValues)[i] = static_cast<uint8_t*>(tempData)[rgbIterator++];
				break;
			case 3:
				((uint8_t*)textureToFill.m_compiledTextureData.compiledColorValues)[i] = 255;
				break;
			}
		}
		delete[] tempData;
	}

	break;
	case TEXTURE_TYPE::R8_SRGB:
		Read8BitStream(inStream, &textureToFill.m_compiledTextureData.compiledColorValues, GetTextureByteSize(textureToFill.m_compiledTextureData.header));
		break;
	case TEXTURE_TYPE::R16G16B16A16:
		Read16BitStream(inStream, &textureToFill.m_compiledTextureData.compiledColorValues, GetTextureByteSize(textureToFill.m_compiledTextureData.header));
		break;
	case TEXTURE_TYPE::R16G16B16:
		Read16BitStream(inStream, &textureToFill.m_compiledTextureData.compiledColorValues, GetTextureByteSize(textureToFill.m_compiledTextureData.header));
		break;
	case TEXTURE_TYPE::R16:
		Read16BitStream(inStream, &textureToFill.m_compiledTextureData.compiledColorValues, GetTextureByteSize(textureToFill.m_compiledTextureData.header));
		break;
	case TEXTURE_TYPE::R32G32B32A32:
		Read32UintBitStream(inStream, &textureToFill.m_compiledTextureData.compiledColorValues, GetTextureByteSize(textureToFill.m_compiledTextureData.header));
		break;
	case TEXTURE_TYPE::R32G32B32:
		Read32UintBitStream(inStream, &textureToFill.m_compiledTextureData.compiledColorValues, GetTextureByteSize(textureToFill.m_compiledTextureData.header));
		break;
	case TEXTURE_TYPE::R32:
		Read32UintBitStream(inStream, &textureToFill.m_compiledTextureData.compiledColorValues, GetTextureByteSize(textureToFill.m_compiledTextureData.header));
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

	inStream.CloseFile();

	textureToFill.m_compiledTextureData.loadState = TEXTURE_LOADSTATE::LOADED_TO_RAM;
	textureToFill.textureName = textureName;
	return true;
}

bool TextureManager::CompileTexture(const char* textureName)
{
	RecursiveFileIterator fileIterator = RecursiveFileIterator(TEXTURES_DIR_IN);
	bool foundTexture = false;

	FilePath currentPath;
	String256 filenameStr;
	while (fileIterator.IterateOverFolderRecursively())
	{
		currentPath = fileIterator.GetCurrentPath();
		if (currentPath.IsFile())
		{
			currentPath = fileIterator.GetCurrentPath();
			const FileObject& currentFileObject = currentPath.Object();
			FromWCharToConstChar(currentFileObject.Name(), filenameStr, 256);
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
		Debug_PrintConsoleConstChar(String256::Format("Could not find texture : %s", textureName));
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

	TextureResource textureToFill;
	inStream.Read((char*)&textureToFill.m_compiledTextureData.header, sizeof(TextureHeader));
	inStream.Seek(GetTextureByteSize(textureToFill.m_compiledTextureData.header), 1);

	const uint64 sizeLeft = inStream.GetFileSize() - inStream.GetFileSeekPosition();
	if (sizeLeft != 0)
	{
		metaResourceToFill.Deserialize(inStream);
	}
}

void Hail::TextureManager::CreateDefaultTexture()
{
	const uint8 widthHeight = 16;
	m_defaultTexture.index = MAX_UINT;
	m_defaultTexture.textureName = "Default Texture";
	m_defaultTexture.m_compiledTextureData.header.height = widthHeight;
	m_defaultTexture.m_compiledTextureData.header.width = widthHeight;
	m_defaultTexture.m_compiledTextureData.header.textureType = (uint32)TEXTURE_TYPE::R8G8B8A8_SRGB;
	m_defaultTexture.m_compiledTextureData.compiledColorValues = new uint8[GetTextureByteSize(m_defaultTexture.m_compiledTextureData.header)];
	m_defaultTexture.m_compiledTextureData.loadState = TEXTURE_LOADSTATE::LOADED_TO_RAM;
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
					memcpy(&((uint8*)m_defaultTexture.m_compiledTextureData.compiledColorValues)[x + y * widthHeight], &color1, sizeof(Color));
				else
					memcpy(&((uint8*)m_defaultTexture.m_compiledTextureData.compiledColorValues)[x + y * widthHeight], &color2, sizeof(Color));
			}
			else
			{
				if ((yIndexValue == 32 || yIndexValue == 48))
					memcpy(&((uint8*)m_defaultTexture.m_compiledTextureData.compiledColorValues)[x + y * widthHeight], &color1, sizeof(Color));
				else
					memcpy(&((uint8*)m_defaultTexture.m_compiledTextureData.compiledColorValues)[x + y * widthHeight], &color2, sizeof(Color));
			}

		}
	}
	CreateTextureInternal(m_defaultTexture, true);
}