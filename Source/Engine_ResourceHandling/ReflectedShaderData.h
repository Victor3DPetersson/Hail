#pragma once

#include "String.hpp"
#include "Types.h"
#include "Containers\VectorOnStack\VectorOnStack.h"
#include "Containers\StaticArray\StaticArray.h"
#include "Resources_Materials\Materials_Common.h"

namespace Hail
{
	struct ShaderDecoration
	{
		uint16 m_bindingLocation;
		uint16 m_elementCount;
		uint32 m_byteSize;
		eShaderValueType m_valueType;
		eDecorationType m_type;
		uint8 m_set;
	};

	static bool operator==(const ShaderDecoration& other, const ShaderDecoration& otherB)
	{
		if (otherB.m_bindingLocation == other.m_bindingLocation ||
			otherB.m_elementCount == other.m_elementCount ||
			otherB.m_byteSize == other.m_byteSize ||
			otherB.m_valueType == other.m_valueType ||
			otherB.m_type == other.m_type ||
			otherB.m_set == other.m_set)
		{
			return true;
		}
		return false;
	}

	struct SetDecorations
	{
		VectorOnStack<ShaderDecoration, 8> m_sampledImages;
		VectorOnStack<ShaderDecoration, 8> m_images;
		VectorOnStack<ShaderDecoration, 8> m_samplers;
		VectorOnStack<ShaderDecoration, 8> m_uniformBuffers;
		VectorOnStack<ShaderDecoration, 8> m_storageBuffers;
	};

	struct ReflectedShaderData
	{
		VectorOnStack<ShaderDecoration, 8> m_shaderInputs;
		VectorOnStack<ShaderDecoration, 8> m_shaderOutputs;
		VectorOnStack<ShaderDecoration, 8> m_pushConstants;
		StaticArray<SetDecorations, (uint32)eDecorationSets::Count> m_setDecorations;
		GrowingArray<ShaderDecoration> m_globalMaterialDecorations;
	};
}