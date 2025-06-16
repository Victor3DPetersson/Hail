#pragma once
#include <stdint.h>
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"
#include "ReflectedShaderData.h"
#include "MetaResource.h"

namespace Hail
{
	enum class eShaderStage : uint8
	{
		Compute,
		Vertex,
		Fragment,
		Amplification,
		Mesh,
		None
	};

	constexpr uint32 ComputeShaderStage = 1 << (uint32)eShaderStage::Compute;
	constexpr uint32 VertexFragmentShaderStage = (1 << (uint32)eShaderStage::Vertex) | (1 << (uint32)eShaderStage::Fragment);

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
		uint32_t shaderType = 0; // ShaderStage, rename in another MR
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
