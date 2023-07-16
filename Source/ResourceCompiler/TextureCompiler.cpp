#include "ResourceCompiler_PCH.h"
#include "TextureCompiler.h"

#include <filesystem>

#include "DebugMacros.h"

#include <iostream>
#include <fstream>

bool IsPowerOfTwo(uint32_t x)
{
	return (x != 0) && ((x & (x - 1)) == 0);
}

struct TGAHEADER
{
	// Length of id string
	char	idLength;

	// Image storage info
	char	colourMapType;
	char	imageType;

	// Colour Map
	short	firstEntry;
	short	numEntries;
	char	bitsPerEntry;

	// Image description
	short	xOrigin;
	short	yOrigin;
	short	width;
	short	height;
	char	bitsPerPixel;
	char	descriptor;
};

enum
{
	TGA_UNSUPPORTED = 1,
	TGA_NO_IMAGE = 2,
	TGA_MAP = 4,
	TGA_RGB = 8,
	TGA_BW = 16,
	TGA_RLE = 32
};

typedef union PixelInfo
{
    uint32_t Colour;
    struct
    {
        uint8_t R, G, B, A;
    };
} *PPixelInfo;

void TextureCompiler::CompileAllTextures()
{


}

bool ExportCompiled8BitTexture(const char* textureName, uint8_t* compiledTextureData, Hail::TextureHeader shaderHeader, uint32_t numberOfColors);
bool ExportCompiled16BitTexture(const char* textureName, uint16_t* compiledTextureData, Hail::TextureHeader shaderHeader, uint32_t numberOfColors) { return false; };
bool ExportCompiled32BitTexture(const char* textureName, uint32_t* compiledTextureData, Hail::TextureHeader shaderHeader, uint32_t numberOfColors){ return false; };

bool TextureCompiler::CompileAndExportAllRequiredTextures(const char** requiredTextures, uint32_t numberOfRequiredTextures)
{
	std::filesystem::path pathToShow{ TEXTURES_DIR_IN };
	Debug_PrintConsoleString256(String256(pathToShow.string().c_str()));

	GrowingArray<std::filesystem::directory_entry> texturesToCompile(numberOfRequiredTextures);
	for (auto& entry : std::filesystem::directory_iterator(pathToShow)) {
		const auto filenameStr = entry.path().filename().replace_extension().string();
		if (entry.is_directory())
		{
			//DEBUG_PRINT_CONSOLE_STRING(String256::Format("%s%s", "\tdir:  ", filenameStr));
		}
		else if (entry.is_regular_file())
		{
			for (uint32_t i = 0; i < numberOfRequiredTextures; i++)
			{
				if (strcmp(requiredTextures[i], filenameStr.c_str()) == 0)
				{
					//DEBUG_PRINT_CONSOLE_STRING(String256::Format("%s%s", "\tfile: ", filenameStr.c_str()));
					const String64 extension = entry.path().extension().string().c_str();
					if (extension == String64(".tga") || extension == String64(".TGA"))
					{
						texturesToCompile.Add(entry);
					}
				}
			}
		}
	}
	if (texturesToCompile.Size() < numberOfRequiredTextures)
	{
		Debug_PrintConsoleConstChar("Missing a required texture in Texture Folder.");
		return false;
	}

	for (uint32_t i = 0; i < numberOfRequiredTextures; i++)
	{
		const String256 textureName = texturesToCompile[i].path().filename().replace_extension().string();
		if (!CompileSpecificTGATexture(texturesToCompile[i].path().string().c_str(), textureName.Data()))
		{
			return false;
		}
	}
	return true;
}

bool TextureCompiler::CompileInternalTexture(Hail::TextureHeader header, const char* textureName)
{
	uint32_t sizeOfColor, numberOfColors;
	switch (ToEnum<Hail::TEXTURE_TYPE>(header.textureType))
	{
	case Hail::TEXTURE_TYPE::R8G8B8A8_SRGB:
		sizeOfColor = 1;
		numberOfColors = 4;
		break;
	case Hail::TEXTURE_TYPE::R8G8B8_SRGB:
		sizeOfColor = 1;
		numberOfColors = 3;
		break;
	case Hail::TEXTURE_TYPE::R8_SRGB:
		sizeOfColor = 1;
		numberOfColors = 1;
		break;
	case Hail::TEXTURE_TYPE::R16G16B16A16:
		sizeOfColor = 2;
		numberOfColors = 4;
		break;
	case Hail::TEXTURE_TYPE::R16G16B16:
		sizeOfColor = 2;
		numberOfColors = 3;
		break;
	case Hail::TEXTURE_TYPE::R16:
		sizeOfColor = 2;
		numberOfColors = 1;
		break;
	case Hail::TEXTURE_TYPE::R32G32B32A32F:
	case Hail::TEXTURE_TYPE::R32G32B32A32:
		sizeOfColor = 4;
		numberOfColors = 4;
		break;
	case Hail::TEXTURE_TYPE::R32G32B32F:
	case Hail::TEXTURE_TYPE::R32G32B32:
		sizeOfColor = 4;
		numberOfColors = 3;
		break;
	case Hail::TEXTURE_TYPE::R32F:
	case Hail::TEXTURE_TYPE::R32:
		sizeOfColor = 4;
		numberOfColors = 1;
		break;
	default:
		return false;
		break;
	}
	return false;
}

bool TextureCompiler::CompileSpecificTGATexture(const char* path, const char* textureName)
{
	TGAHEADER tgaHeader;

	// Holds bitwise flags for TGA file
	int tgaDesc = 0;

	std::ifstream tgaFile(path, std::ios::in | std::ios::binary);

	if (!tgaFile.is_open())
	{
		Debug_PrintConsoleString256(String256::Format("Error opening: %s", path));
		return false;
	}

	// Go to end of file to check TGA version
	tgaFile.seekg(0, std::ios::end);

	// We need to store the file size for a worst case scenario
	// RLE compression can increase the amount of data
	// depending on the image. (This scenario will only arise, in
	// an image with very few same pixel runs).
	int fileSize = (int)tgaFile.tellg();

	// Seek to version identifier (Always specified as being 18
	// characters from the end of the file)
	tgaFile.seekg(-18, std::ios::end);

	// Read version identifier
	char versionCheck[17] = "";
	tgaFile.read(versionCheck, 16);

	// Check version
	int version = 1;
	if (strcmp(versionCheck, "TRUEVISION-XFILE") == 0)
	{
		version = 2;
	}

	// Back to the beginning of the file
	tgaFile.seekg(0, std::ios::beg);

	// Need to read each field in one at a time since the structure padding likes
	// to eat the 4th and 10th bytes
	tgaFile.read(&tgaHeader.idLength, sizeof(tgaHeader.idLength));
	tgaFile.read(&tgaHeader.colourMapType, sizeof(tgaHeader.colourMapType));
	tgaFile.read(&tgaHeader.imageType, sizeof(tgaHeader.imageType));

	// If colourMapType is 0 and these 3 fields below are not 0, something may have went wrong
	tgaFile.read((char*)(&tgaHeader.firstEntry), sizeof(tgaHeader.firstEntry));
	tgaFile.read((char*)(&tgaHeader.numEntries), sizeof(tgaHeader.numEntries));
	tgaFile.read(&tgaHeader.bitsPerEntry, sizeof(tgaHeader.bitsPerEntry));


	tgaFile.read((char*)(&tgaHeader.xOrigin), sizeof(tgaHeader.xOrigin));
	tgaFile.read((char*)(&tgaHeader.yOrigin), sizeof(tgaHeader.yOrigin));
	tgaFile.read((char*)(&tgaHeader.width), sizeof(tgaHeader.width));
	tgaFile.read((char*)(&tgaHeader.height), sizeof(tgaHeader.height));
	tgaFile.read(&tgaHeader.bitsPerPixel, sizeof(tgaHeader.bitsPerPixel));
	tgaFile.read(&tgaHeader.descriptor, sizeof(tgaHeader.descriptor));

	switch (tgaHeader.imageType)
	{
	case 0:
		tgaDesc |= TGA_NO_IMAGE;
		break;
	case 2:
		tgaDesc |= TGA_RGB;
		break;
	case 10:
		tgaDesc |= (TGA_RGB | TGA_RLE);
		break;
	default:
		tgaDesc |= TGA_UNSUPPORTED;
		break;
	}

	if ((tgaDesc & TGA_UNSUPPORTED) == 0)
	{
		//Debug_PrintConsoleConstChar("TGA Format Supported");
	}
	else
	{
		Debug_PrintConsoleConstChar("TGA Format UnSupported in file: ");
		Debug_PrintConsoleConstChar(textureName);
		return false;
	}
	char* skip = "";
	tgaFile.read(skip, tgaHeader.idLength);

	// Skip the colour map if it doesn't exist
	if (!(tgaDesc & TGA_MAP))
	{
		int colourMapSize = tgaHeader.colourMapType * tgaHeader.numEntries;
		tgaFile.read(skip, colourMapSize);
	}

	//if (!IsPowerOfTwo(tgaHeader.height) || !IsPowerOfTwo(tgaHeader.width))
	//{
	//	Debug_PrintConsoleConstChar("Texture is not in Power of 2");
	//	return false;
	//}

	// imageDataSize is the total number of bytes taken by the image
	// after being loaded and decompressed if necessary
	int imageDataSize = tgaHeader.width * tgaHeader.height * (tgaHeader.bitsPerPixel / 8);
	
	unsigned char* pixelData = new unsigned char[imageDataSize];

	// Read the image data
	int originalPosition = (int)tgaFile.tellg();

	// This read operation may read past the end of the file
	// so could break something (hasn't happened yet)
	tgaFile.read((char*)pixelData, imageDataSize);

	// RLE decoding
	if (tgaDesc & TGA_RLE)
	{
		Debug_PrintConsoleConstChar("Decoding RLE");
		// Used to decode RLE
		char* tempPixelData;
		tempPixelData = new char[fileSize];
		// Copy data over for decoding
		memcpy(tempPixelData, pixelData, fileSize);
		// Holds the current pixel index for the j loop below
		int indexAccum = 0;
		int bytesPerPixel = (tgaHeader.bitsPerPixel / 8);
		int bytesPerPixelRLE = bytesPerPixel + 1;

		// Increments of i are controlled in the for loop because depending
		// on whether or not the packet being checked is run-length encoded 
		// the increment may have to be between bytesPerPixel and 128 (Max size of either packet)
		for (int i = 0; indexAccum < imageDataSize; )
		{
			// runCount holds the length of the packet taken from the packet info byte
			// runCount can be a maximum of 127.
			int runCount = (127 & tempPixelData[i]) + 1;

			// Check the packet info byte for RLE
			// Run-length encoded packet
			if (128 & tempPixelData[i])
			{
				// In an encoded packet, runCount specifies
				// the repititions of the pixel data (up to 127)
				for (int j = 0; j < runCount; j++)
				{
					for (int k = 1; k < bytesPerPixelRLE; k++)
						pixelData[indexAccum++] = tempPixelData[i + k];
				}

				i += bytesPerPixelRLE;
			}

			// Raw data packet
			else if (!(128 & tempPixelData[i]))
			{
				// Skip past the packet info byte
				i++;

				// In a raw packet, runCount specifies
				// the number of pixels that are to follow (up to 127)
				for (int j = 0; j < runCount; j++)
				{
					for (int k = 0; k < bytesPerPixel; k++)
						pixelData[indexAccum++] = tempPixelData[i + k];

					i += bytesPerPixel;
				}
			}
		}

		delete[] tempPixelData;
	}

	//Flipping texture in Y Coord for Vulkan Renderdoc became annoying, but better to work from correct space
	//unsigned char* tempPixelData = new unsigned char[imageDataSize];
	//memcpy(tempPixelData, pixelData, imageDataSize);
	//const uint8_t bytesPerPixel = (tgaHeader.bitsPerPixel / 8);
	//const uint32_t bytesPerRow = tgaHeader.width * bytesPerPixel;
	//for (uint32_t row = 0; row < tgaHeader.height; row++)
	//{
	//	const uint32_t readFromLocation = ((tgaHeader.height - 1) - row) * bytesPerRow;
	//	const uint32_t writeToLocation = row * bytesPerRow;
	//	memcpy(pixelData + writeToLocation, tempPixelData + readFromLocation, bytesPerRow);
	//}
	//delete[] tempPixelData;

	Hail::TextureHeader compileHeader;
	uint32_t numberOfColors = 0;
	if (tgaHeader.bitsPerPixel / 8 == 3)
	{
		compileHeader.textureType = ToUnderlyingType<Hail::TEXTURE_TYPE>(Hail::TEXTURE_TYPE::R8G8B8_SRGB);
		numberOfColors = 3;
	}
	else if (tgaHeader.bitsPerPixel / 8 == 4)
	{
		compileHeader.textureType = ToUnderlyingType<Hail::TEXTURE_TYPE>(Hail::TEXTURE_TYPE::R8G8B8A8_SRGB);
		numberOfColors = 4;
	}
	compileHeader.height = tgaHeader.height;
	compileHeader.width = tgaHeader.width;

	if (!ExportCompiled8BitTexture(textureName, pixelData, compileHeader, numberOfColors))
	{
		return false;
	}

	return true;
}


bool TextureCompiler::IsTextureCompiled(const char* relativePath, const char* textureName)
{
	return false;
}


void TextureCompiler::Init()
{

}


bool ExportCompiled8BitTexture(const char* textureName, uint8_t* compiledTextureData, Hail::TextureHeader textureHeader, uint32_t numberOfColors)
{
	Debug_PrintConsoleString256(String256::Format("\nExporting Texture:\n%s:", textureName));
	Debug_PrintConsoleString256(String256::Format("Texture Width:%i Heigth:%i :%s", textureHeader.width, textureHeader.height, "\n"));

	if (std::filesystem::exists(TEXTURES_DIR_OUT) == false)
	{
		std::filesystem::create_directory(TEXTURES_DIR_OUT);
	}

	String256 outPath = String256::Format("%s%s%s", TEXTURES_DIR_OUT, textureName, ".txr");
	std::ofstream outStream(outPath.Data(), std::ios::out | std::ios::binary);
	if (!outStream)
	{
		Debug_PrintConsoleConstChar("ERROR: Failed to export^");
		return false;
	}
	outStream.write((char*)&textureHeader, sizeof(textureHeader));

	for (uint32_t i = 0; i < textureHeader.width * textureHeader.height * numberOfColors; i+= numberOfColors)
	{
		switch (numberOfColors)
		{
		case 1:
			outStream.write((char*)&compiledTextureData[i], sizeof(char));
			break;
		case 2:
			outStream.write((char*)&compiledTextureData[i], sizeof(char));
			outStream.write((char*)&compiledTextureData[i + 1], sizeof(char));
			break;
		case 3:
			outStream.write((char*)&compiledTextureData[i + 2], sizeof(char)); //flipping to rgb from bgr
			outStream.write((char*)&compiledTextureData[i + 1], sizeof(char));
			outStream.write((char*)&compiledTextureData[i], sizeof(char));
			break;
		case 4:
			outStream.write((char*)&compiledTextureData[i + 2], sizeof(char)); //flipping to rgb from bgr
			outStream.write((char*)&compiledTextureData[i + 1], sizeof(char));
			outStream.write((char*)&compiledTextureData[i], sizeof(char));
			outStream.write((char*)&compiledTextureData[i + 3], sizeof(char));
			break;
		default:
			break;
		}
	}
	//outStream.write((char*)&compiledTextureData, textureHeader.width * textureHeader.height * sizeOfColor * numberOfColors);
	outStream.close();
	return true;
}


