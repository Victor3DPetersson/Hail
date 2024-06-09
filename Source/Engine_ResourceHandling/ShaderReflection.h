#pragma once
#include "Types.h"
#include "ReflectedShaderData.h"
namespace Hail
{
	void ParseShader(ReflectedShaderData& returnData, const char* shaderName, const char* compiledShader, uint32 shaderLength);
}