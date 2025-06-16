#pragma once
#include "Types.h"
#include "ReflectedShaderData.h"
namespace Hail
{
	void ParseShader(ReflectedShaderData& returnData, eShaderStage shaderStage, const char* shaderName, const char* compiledShader, uint32 shaderLength);
}