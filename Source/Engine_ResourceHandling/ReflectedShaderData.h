#pragma once

#include "String.hpp"
#include "Types.h"
#include "Containers\VectorOnStack\VectorOnStack.h"
#include "Containers\StaticArray\StaticArray.h"
#include "Resources_Materials\Materials_Common.h"

namespace Hail
{
	constexpr uint32 MaxShaderBindingCount = 16u;
	struct ShaderDecoration
	{
		uint16 m_bindingLocation = MAX_UINT16;
		uint16 m_elementCount = MAX_UINT16;
		uint32 m_byteSize = MAX_UINT;
		eShaderValueType m_valueType = eShaderValueType::none;
		eDecorationType m_type = eDecorationType::Count;
		uint8 m_set = 0xffff;
		eShaderAccessQualifier m_accessQualifier;
		uint16 m_flags = 0u; // For images it is used to differentiate between storage images and sampled images for read only textures
	};

	static bool operator==(const ShaderDecoration& other, const ShaderDecoration& otherB)
	{
		if (otherB.m_bindingLocation == other.m_bindingLocation &&
			otherB.m_elementCount == other.m_elementCount &&
			otherB.m_byteSize == other.m_byteSize &&
			otherB.m_valueType == other.m_valueType &&
			otherB.m_type == other.m_type &&
			otherB.m_set == other.m_set &&
			otherB.m_flags == other.m_flags)
		{
			return true;
		}
		return false;
	}

	//TODO: Replace this struct with a hasmap like structure
	struct SetDecoration
	{
		StaticArray<ShaderDecoration, MaxShaderBindingCount> m_decorations;
		VectorOnStack<uint32, MaxShaderBindingCount> m_indices;
	};

	struct EntryDecorations
	{
		glm::uvec3 workGroupSize;
	};

	struct ReflectedShaderData
	{
		VectorOnStack<ShaderDecoration, 8> m_shaderInputs;
		VectorOnStack<ShaderDecoration, 8> m_shaderOutputs;
		VectorOnStack<ShaderDecoration, 8> m_pushConstants;
		StaticArray<StaticArray<SetDecoration, (uint32)eDecorationType::Count>, (uint32)eDecorationSets::Count> m_setDecorations;
		GrowingArray<ShaderDecoration> m_globalMaterialDecorations;
		EntryDecorations m_entryDecorations;
		bool m_bIsValid;
	};
}