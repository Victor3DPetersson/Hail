#include "Engine_PCH.h"
#include "TextureManager.h"

#include "DebugMacros.h"
#include "TextureCompiler.h"

#include "Utility\FileSystem.h"
#include "Utility\StringUtility.h"
#include "Utility\InOutStream.h"

namespace
{
	const char* REQUIRED_TEXTURES[REQUIRED_TEXTURE_COUNT] =
	{
		"Debug_Grid",
		"spaceShip",
		"cloud shapes test"
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


namespace Hail
{
	TextureManager::TextureManager()
	{
		//m_compiledRequiredTextures.Init(REQUIRED_TEXTURE_COUNT);
		m_loadedTextures.Init(10);
	}

	void TextureManager::Update()
	{
		//TODO: Add file watcher here
	}

	bool TextureManager::LoadTexture(const char* textureName)
	{
		CompiledTexture outTexture;
		String256 inPath = String256::Format("%s%s%s", TEXTURES_DIR_OUT, textureName, ".txr");

		InOutStream inStream;

		if (!inStream.OpenFile(inPath.Data(), FILE_OPEN_TYPE::READ, true))
		{
			if (!CompileTexture(textureName))
			{
				return false;
			}
		}
		inStream.OpenFile(inPath.Data(), FILE_OPEN_TYPE::READ, true);

		Debug_PrintConsoleString256(String256::Format("\nImporting Texture:\n%s:", textureName));

		inStream.Read((char*)&outTexture.header, sizeof(TextureHeader));
		Debug_PrintConsoleString256(String256::Format("Texture Width:%i Heigth:%i :%s", outTexture.header.width, outTexture.header.height, "\n"));
		switch (ToEnum<TEXTURE_TYPE>(outTexture.header.textureType))
		{
		case TEXTURE_TYPE::R8G8B8A8_SRGB:
			Read8BitStream(inStream, &outTexture.compiledColorValues, GetTextureByteSize(outTexture.header));
			break;
		case TEXTURE_TYPE::R8G8B8_SRGB:
		{
			void* tempData = nullptr;
			Read8BitStream(inStream, &tempData, GetTextureByteSize(outTexture.header));
			outTexture.header.textureType = static_cast<uint32_t>(TEXTURE_TYPE::R8G8B8A8_SRGB);
			outTexture.compiledColorValues = new uint8_t[1 * 4 * outTexture.header.width * outTexture.header.height];
			uint32_t rgbIterator = 0;
			for (size_t i = 0; i < 4 * outTexture.header.width * outTexture.header.height; i++)
			{
				switch (i % 4)
				{
				case 0:
					static_cast<uint8_t*>(outTexture.compiledColorValues)[i] = static_cast<uint8_t*>(tempData)[rgbIterator++];
					break;
				case 1:
					static_cast<uint8_t*>(outTexture.compiledColorValues)[i] = static_cast<uint8_t*>(tempData)[rgbIterator++];
					break;
				case 2:
					static_cast<uint8_t*>(outTexture.compiledColorValues)[i] = static_cast<uint8_t*>(tempData)[rgbIterator++];
					break;
				case 3:
					static_cast<uint8_t*>(outTexture.compiledColorValues)[i] = 255;
					break;
				}
			}
			delete[] tempData;
		}

			break;
		case TEXTURE_TYPE::R8_SRGB:
			Read8BitStream(inStream, &outTexture.compiledColorValues, GetTextureByteSize(outTexture.header));
			break;
		case TEXTURE_TYPE::R16G16B16A16:
			Read16BitStream(inStream, &outTexture.compiledColorValues, GetTextureByteSize(outTexture.header));
			break;
		case TEXTURE_TYPE::R16G16B16:
			Read16BitStream(inStream, &outTexture.compiledColorValues, GetTextureByteSize(outTexture.header));
			break;
		case TEXTURE_TYPE::R16:
			Read16BitStream(inStream, &outTexture.compiledColorValues, GetTextureByteSize(outTexture.header));
			break;
		case TEXTURE_TYPE::R32G32B32A32:
			Read32UintBitStream(inStream, &outTexture.compiledColorValues, GetTextureByteSize(outTexture.header));
			break;
		case TEXTURE_TYPE::R32G32B32:
			Read32UintBitStream(inStream, &outTexture.compiledColorValues, GetTextureByteSize(outTexture.header));
			break;
		case TEXTURE_TYPE::R32:
			Read32UintBitStream(inStream, &outTexture.compiledColorValues, GetTextureByteSize(outTexture.header));
			break;
		default:
			return false;
			break;
		}
		inStream.CloseFile();

		outTexture.loadState = TEXTURE_LOADSTATE::LOADED_TO_RAM;

		TextureResource textureResource{};
		textureResource.m_compiledTextureData = outTexture;
		textureResource.index = m_loadedTextures.Size();
		textureResource.textureName = textureName;
		m_loadedTextures.Add(textureResource);
		return true;
	}

	bool TextureManager::LoadAllRequiredTextures()
	{
		GrowingArray<String256> foundCompiledTextures(REQUIRED_TEXTURE_COUNT);
		RecursiveFileIterator fileIterator = RecursiveFileIterator(TEXTURES_DIR_OUT);

		Debug_PrintConsoleString64(String64("Required Textures:    "));
		{
			FilePath currentPath;
			while (fileIterator.IterateOverFolderRecursively())
			{
				currentPath = fileIterator.GetCurrentPath();
				if (currentPath.IsFile())
				{
					const FileObject& currentFileObject = currentPath.Object();
					String256 filenameStr;
					FromWCharToConstChar(currentFileObject.Name(), filenameStr.Data(), 256);
					Debug_PrintConsoleString256(String256::Format("%s%s", "\tfile: ", filenameStr));
					foundCompiledTextures.Add(filenameStr);
				}

			}
		}


		uint32_t foundCounter = 0;
		for (uint32_t texture = 0; texture < foundCompiledTextures.Size(); texture++)
		{
			bool foundTexture = false;
			for (uint32_t i = 0; i < REQUIRED_TEXTURE_COUNT; i++)
			{
				if (StringCompare(REQUIRED_TEXTURES[i], foundCompiledTextures[texture].Data()))
				{
					foundTexture = true;
					foundCounter++;
				}
			}
			if (!foundTexture)
			{
				foundCompiledTextures.RemoveCyclicAtIndex(texture);
				texture--;
			}
		}
		if (foundCounter == REQUIRED_TEXTURE_COUNT)
		{
			for (size_t i = 0; i < REQUIRED_TEXTURE_COUNT; i++)
			{
				if (!LoadTexture(foundCompiledTextures[i]))
				{
					return false;
				}
			}
			return true;
		}
		return CompileRequiredTextures();
	}

	GrowingArray<TextureResource>* TextureManager::GetTextures()
	{
		if (m_loadedTextures.IsInitialized() && m_loadedTextures.Size() == REQUIRED_TEXTURE_COUNT)
		{
			return  &m_loadedTextures;
		}
		return nullptr;
	}

	bool TextureManager::CompileRequiredTextures()
	{
		bool compilationSuccess = TextureCompiler::CompileAndExportAllRequiredTextures(REQUIRED_TEXTURES, REQUIRED_TEXTURE_COUNT);
		if (compilationSuccess)
		{
			for (size_t i = 0; i < REQUIRED_TEXTURE_COUNT; i++)
			{
				TextureResource textureResource;
				CompiledTexture texture;
				if (!LoadTexture(REQUIRED_TEXTURES[i]))
				{
					return false;
				}
			}
		}
		return compilationSuccess;
	}

	bool TextureManager::CompileTexture(const char* textureName)
	{
		RecursiveFileIterator fileIterator = RecursiveFileIterator(TEXTURES_DIR_IN);
		bool foundTexture = false;

		FilePath currentPath;
		String256 filenameStr;
		while (fileIterator.IterateOverFolderRecursively())
		{
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

		return TextureCompiler::CompileSpecificTGATexture(currentPath);
	}


	void TextureManager::Cleanup()
	{
		m_loadedTextures.DeleteAll();
	}
}
