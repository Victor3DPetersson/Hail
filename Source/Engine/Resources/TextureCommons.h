#pragma once
#include <stdint.h>
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"
#include "DebugMacros.h"
#include "../EngineConstants.h"

namespace Hail
{
	enum class TEXTURE_TYPE : uint32_t
	{
		R32G32B32A32F,
		R32G32B32F,
		R32F,
		R32G32B32A32,
		R32G32B32,
		R32,
		R16G16B16A16,
		R16G16B16,
		R16,
		R8G8B8A8,
		R8G8B8,
		R8,
	};

	struct TextureHeader
	{
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t textureType = 0;
	};

	enum class TEXTURE_LOADSTATE
	{
		UNLOADED,
		LOADED_TO_RAM,
		UPLOADED_TO_GPU
	};

	struct CompiledTexture
	{
		TextureHeader header;
		void* compiledColorValues = nullptr;
		TEXTURE_LOADSTATE loadState = TEXTURE_LOADSTATE::UNLOADED;
	};

	void DeleteCompiledTexture(CompiledTexture& texture);


	uint32_t GetTextureByteSize(TextureHeader header);
}
