#include "Engine_PCH.h"
#include "TextureManager.h"

#include <filesystem>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "DebugMacros.h"
#include "TextureCompiler.h"


const char* REQUIRED_TEXTURES[REQUIRED_TEXTURE_COUNT] =
{
	"Debug_Grid",
};


TextureManager::TextureManager()
{
	m_compiledRequiredTextures.Init(REQUIRED_TEXTURE_COUNT);
}

void TextureManager::Update()
{
	//TODO: Add file watcher here
}

//TODO: Add relative path support in the shader output
bool TextureManager::LoadTexture(const char* textureName, CompiledTexture& outTexture)
{
	String256 inPath = String256::Format("%s%s%s", TEXTURES_DIR_OUT, textureName, ".txr");

	std::ifstream inStream(inPath.Data(), std::ios::in | std::ios::binary);
	if (!inStream)
	{
		return false;
	}

	Debug_PrintConsoleString256(String256::Format("\nImporting Texture:\n%s:", textureName));

	inStream.read((char*)&outTexture.header, sizeof(TextureHeader));
	Debug_PrintConsoleString256(String256::Format("Texture Width:%i Heigth:%i :%s", outTexture.header.width, outTexture.header.height, "\n"));
	TextureHeader& header = outTexture.header;

	uint32_t width, heigth, numberOfColors, byteSizePixel;
	width = header.width;
	heigth = header.height;
	numberOfColors = 0;
	byteSizePixel = 0;

	switch (ToEnum<TEXTURE_TYPE>(outTexture.header.textureType))
	{
	case TEXTURE_TYPE::R8G8B8A8:
	{
		byteSizePixel = 1;
		numberOfColors = 4;
		outTexture.compiledColorValues = new unsigned char[width * heigth * numberOfColors];

		const uint32_t imageByteSize = byteSizePixel * numberOfColors * width * heigth;
		unsigned char* pixels = (unsigned char*)malloc(imageByteSize);
		inStream.read((char*)&pixels[0], imageByteSize);
		memcpy(outTexture.compiledColorValues, pixels, imageByteSize);
		free(pixels);
	}
		break;
	case TEXTURE_TYPE::R8G8B8:
	{
		byteSizePixel = 1;
		numberOfColors = 3;
		outTexture.compiledColorValues = new unsigned char[width * heigth * numberOfColors];

		const uint32_t imageByteSize = byteSizePixel * numberOfColors * width * heigth;
		unsigned char* pixels = (unsigned char*)malloc(imageByteSize);
		inStream.read((char*)&pixels[0], imageByteSize);
		memcpy(outTexture.compiledColorValues, pixels, imageByteSize);
		free(pixels);
	}

		break;
	case TEXTURE_TYPE::R8:
	{
		byteSizePixel = 1;
		numberOfColors = 1;
		outTexture.compiledColorValues = new unsigned char[width * heigth * numberOfColors];

		const uint32_t imageByteSize = byteSizePixel * numberOfColors * width * heigth;
		unsigned char* pixels = (unsigned char*)malloc(imageByteSize);
		inStream.read((char*)&pixels[0], imageByteSize);
		memcpy(outTexture.compiledColorValues, pixels, imageByteSize);
		free(pixels);
	}

		break;
	case TEXTURE_TYPE::R16G16B16A16:
	{
		byteSizePixel = 2;
		numberOfColors = 4;
		outTexture.compiledColorValues = new uint16_t[width * heigth * numberOfColors];

		const uint32_t imageByteSize = byteSizePixel * numberOfColors * width * heigth;
		uint16_t* pixels = (uint16_t*)malloc(imageByteSize);
		inStream.read((char*)&pixels[0], imageByteSize);
		memcpy(outTexture.compiledColorValues, pixels, imageByteSize);
		free(pixels);
	}

		break;
	case TEXTURE_TYPE::R16G16B16:
	{
		byteSizePixel = 2;
		numberOfColors = 3;
		outTexture.compiledColorValues = new uint16_t[width * heigth * numberOfColors];

		const uint32_t imageByteSize = byteSizePixel * numberOfColors * width * heigth;
		uint16_t* pixels = (uint16_t*)malloc(imageByteSize);
		inStream.read((char*)&pixels[0], imageByteSize);
		memcpy(outTexture.compiledColorValues, pixels, imageByteSize);
		free(pixels);
	}
		break;
	case TEXTURE_TYPE::R16:
	{
		byteSizePixel = 2;
		numberOfColors = 1;
		outTexture.compiledColorValues = new uint16_t[width * heigth * numberOfColors];

		const uint32_t imageByteSize = byteSizePixel * numberOfColors * width * heigth;
		uint16_t* pixels = (uint16_t*)malloc(imageByteSize);
		inStream.read((char*)&pixels[0], imageByteSize);
		memcpy(outTexture.compiledColorValues, pixels, imageByteSize);
		free(pixels);
	}

		break;
	case TEXTURE_TYPE::R32G32B32A32:
	{
		byteSizePixel = 4;
		numberOfColors = 4;
		outTexture.compiledColorValues = new uint32_t[width * heigth * numberOfColors];

		const uint32_t imageByteSize = byteSizePixel * numberOfColors * width * heigth;
		uint32_t* pixels = (uint32_t*)malloc(imageByteSize);
		inStream.read((char*)&pixels[0], imageByteSize);
		memcpy(outTexture.compiledColorValues, pixels, imageByteSize);
		free(pixels);
	}
		break;
	case TEXTURE_TYPE::R32G32B32:
	{
		byteSizePixel = 4;
		numberOfColors = 3;
		outTexture.compiledColorValues = new uint32_t[width * heigth * numberOfColors];

		const uint32_t imageByteSize = byteSizePixel * numberOfColors * width * heigth;
		uint32_t* pixels = (uint32_t*)malloc(imageByteSize);
		inStream.read((char*)&pixels[0], imageByteSize);
		memcpy(outTexture.compiledColorValues, pixels, imageByteSize);
		free(pixels);
	}
		break;
	case TEXTURE_TYPE::R32:
	{
		byteSizePixel = 4;
		numberOfColors = 1;
		outTexture.compiledColorValues = new uint32_t[width * heigth * numberOfColors];

		const uint32_t imageByteSize = byteSizePixel * numberOfColors * width * heigth;
		uint32_t* pixels = (uint32_t*)malloc(imageByteSize);
		inStream.read((char*)&pixels[0], imageByteSize);
		memcpy(outTexture.compiledColorValues, pixels, imageByteSize);
		free(pixels);
	}
		break;
	default:
		return false;
		break;
	}
	inStream.close();

	outTexture.loadState = TEXTURE_LOADSTATE::LOADED_TO_RAM;
	outTexture.textureName = textureName;
	return true;
}

bool TextureManager::LoadAllRequiredTextures()
{
	GrowingArray<String256> foundCompiledTextures(REQUIRED_TEXTURE_COUNT);
	std::filesystem::path pathToShow{ TEXTURES_DIR_OUT };
	Debug_PrintConsoleString64(String64("Required Textures:    "));
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
			CompiledTexture texture;
			if (!LoadTexture(foundCompiledTextures[i], texture))
			{
				return false;
			}
			m_compiledRequiredTextures.Add(texture);
		}
		return true;
	}
	return CompileRequiredTextures();
}

GrowingArray<CompiledTexture>* TextureManager::GetRequiredTextures()
{
	if (m_compiledRequiredTextures.IsInitialized() && m_compiledRequiredTextures.Size() == REQUIRED_TEXTURE_COUNT)
	{
		return  &m_compiledRequiredTextures;
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
			CompiledTexture texture;
			if (!LoadTexture(REQUIRED_TEXTURES[i], texture))
			{
				return false;
			}
			m_compiledRequiredTextures.Add(texture);
		}
	}
	return compilationSuccess;
}


void TextureManager::Cleanup()
{
	m_compiledRequiredTextures.DeleteAll();
}