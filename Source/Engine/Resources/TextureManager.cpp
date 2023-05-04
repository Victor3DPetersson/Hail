#include "Engine_PCH.h"
#include "TextureManager.h"

#include <filesystem>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "DebugMacros.h"
#include "TextureCompiler.h"

namespace
{
	const char* REQUIRED_TEXTURES[REQUIRED_TEXTURE_COUNT] =
	{
		"Debug_Grid",
		"spaceShip",
	};
	void Read8BitStream(std::ifstream& stream, void** outData, const uint32_t numberOfBytesToRead)
	{
		*outData = new uint8_t[numberOfBytesToRead];
		unsigned char* pixels = (unsigned char*)malloc(numberOfBytesToRead);
		stream.read((char*)&pixels[0], numberOfBytesToRead);
		memcpy(*outData, pixels, numberOfBytesToRead);
		free(pixels);
	}
	void Read16BitStream(std::ifstream& stream, void** outData, const uint32_t numberOfBytesToRead)
	{
		*outData = new uint16_t[numberOfBytesToRead];
		uint16_t* pixels = (uint16_t*)malloc(numberOfBytesToRead);
		stream.read((char*)&pixels[0], numberOfBytesToRead);
		memcpy(*outData, pixels, numberOfBytesToRead);
		free(pixels);
	}
	void Read32UintBitStream(std::ifstream& stream, void** outData, const uint32_t numberOfBytesToRead)
	{
		*outData = new uint32_t[numberOfBytesToRead];
		uint32_t* pixels = (uint32_t*)malloc(numberOfBytesToRead);
		stream.read((char*)&pixels[0], numberOfBytesToRead);
		memcpy(*outData, pixels, numberOfBytesToRead);
		free(pixels);
	}
	void Read32FltBitStream(std::ifstream& stream, void** outData, const uint32_t numberOfBytesToRead)
	{
		*outData = new float[numberOfBytesToRead];
		float* pixels = (float*)malloc(numberOfBytesToRead);
		stream.read((char*)&pixels[0], numberOfBytesToRead);
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

		std::ifstream inStream(inPath.Data(), std::ios::in | std::ios::binary);
		if (!inStream)
		{
			if (!CompileTexture(textureName))
			{
				return false;
			}
			inStream.open(inPath.Data(), std::ios::in | std::ios::binary);
		}

		Debug_PrintConsoleString256(String256::Format("\nImporting Texture:\n%s:", textureName));

		inStream.read((char*)&outTexture.header, sizeof(TextureHeader));
		Debug_PrintConsoleString256(String256::Format("Texture Width:%i Heigth:%i :%s", outTexture.header.width, outTexture.header.height, "\n"));
		switch (ToEnum<TEXTURE_TYPE>(outTexture.header.textureType))
		{
		case TEXTURE_TYPE::R8G8B8A8:
			Read8BitStream(inStream, &outTexture.compiledColorValues, GetTextureByteSize(outTexture.header));
			break;
		case TEXTURE_TYPE::R8G8B8:
			Read8BitStream(inStream, &outTexture.compiledColorValues, GetTextureByteSize(outTexture.header));
			break;
		case TEXTURE_TYPE::R8:
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
		inStream.close();

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
		std::filesystem::path pathToShow{ TEXTURES_DIR_OUT };
		Debug_PrintConsoleString64(String64("Required Textures:    "));
		if (std::filesystem::exists(pathToShow))
		{
			for (const auto& entry : std::filesystem::directory_iterator(pathToShow))
			{
				const auto filenameStr = entry.path().filename().replace_extension().string();
				if (entry.is_directory())
				{
					Debug_PrintConsoleString256(String256::Format("%s%s", "\tdir:  ", filenameStr.c_str()));
				}
				else if (entry.is_regular_file())
				{
					Debug_PrintConsoleString256(String256::Format("%s%s", "\tfile: ", filenameStr.c_str()));
					foundCompiledTextures.Add(filenameStr);
				}
			}
		}

		uint32_t foundCounter = 0;
		for (uint32_t shader = 0; shader < foundCompiledTextures.Size(); shader++)
		{
			bool foundShader = false;
			for (uint32_t i = 0; i < REQUIRED_TEXTURE_COUNT; i++)
			{
				if (strcmp(REQUIRED_TEXTURES[i], foundCompiledTextures[shader].Data()) == 0)
				{
					foundShader = true;
					foundCounter++;
				}
			}
			if (!foundShader)
			{
				foundCompiledTextures.RemoveCyclicAtIndex(shader);
				shader--;
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
		std::filesystem::path pathToShow{ TEXTURES_DIR_IN };
		Debug_PrintConsoleString256(String256(pathToShow.string().c_str()));

		bool foundTexture = false;
		String256 finalEntry;
		for (auto& entry : std::filesystem::directory_iterator(pathToShow)) {
			const auto filenameStr = entry.path().filename().replace_extension().string();
			if (entry.is_directory())
			{
				//DEBUG_PRINT_CONSOLE_STRING(String256::Format("%s%s", "\tdir:  ", filenameStr));
			}
			else if (entry.is_regular_file())
			{
				if (strcmp(textureName, filenameStr.c_str()) == 0)
				{
					//DEBUG_PRINT_CONSOLE_STRING(String256::Format("%s%s", "\tfile: ", filenameStr.c_str()));
					const String64 extension = entry.path().extension().string().c_str();
					if (extension == String64(".tga") || extension == String64(".TGA"))
					{
						foundTexture = true;
						finalEntry = entry.path().string().c_str();
						break;
					}
				}
			}
		}

		if (!foundTexture)
		{
			Debug_PrintConsoleConstChar(String256::Format("Could not find texture : %s", textureName));
			return false;
		}

		if (!TextureCompiler::CompileSpecificTGATexture(finalEntry, textureName))
		{
			return false;
		}
		return false;
	}


	void TextureManager::Cleanup()
	{
		m_loadedTextures.DeleteAll();
	}
}
