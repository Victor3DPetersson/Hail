#pragma once
#include <stdint.h>
#include "Containers\GrowingArray\GrowingArray.h"
#include "String.hpp"

namespace Hail
{
	enum class SHADERTYPE
	{
		COMPUTE,
		VERTEX,
		CONTROL,
		EVALUATION,
		FRAGMENT
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

	enum class SHADER_LOADSTATE
	{
		UNLOADED,
		LOADED_TO_RAM,
		UPLOADED_TO_GPU
	};

	struct CompiledShader
	{
		ShaderHeader header;
		char* compiledCode = nullptr;
		SHADER_LOADSTATE loadState = SHADER_LOADSTATE::UNLOADED;
		String64 shaderName;
	};
}
