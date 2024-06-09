#pragma once
#include <stdint.h>
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"
#include "ReflectedShaderData.h"
#include "MetaResource.h"

namespace Hail
{
	enum class eShaderType : uint8
	{
		Compute,
		Vertex,
		Fragment,
		Amplification,
		Mesh,
		None
	};

	enum class SHADERLANGUAGETARGET
	{
		GLSL,
		HLSL,
		METAL,
		COUNT
	};

	struct ShaderHeader
	{
		uint32_t sizeOfShaderData = 0;
		uint32_t shaderType = 0;
	};

	enum class eShaderLoadState
	{
		Unloaded,
		LoadedToRAM
	};

	struct CompiledShader
	{
		ShaderHeader header;
		MetaResource m_metaData;
		char* compiledCode = nullptr;
		eShaderLoadState loadState = eShaderLoadState::Unloaded;
		String64 shaderName;
		ReflectedShaderData reflectedShaderData;
		uint64 m_nameHash{};
	};
}
