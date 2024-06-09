#pragma once

#include "Types.h"
#include "Containers\VectorOnStack\VectorOnStack.h"
#include "Containers\StaticArray\StaticArray.h"

// Contains platform agnostic and shared Material structs
namespace Hail
{
	enum class eBlendMode : uint8
	{
		None,
		Cutout,
		Translucent,
		Additive,
		COUNT
	};

	// A material type owns the the descriptors that holds set 0 and 1 data and buffers
	enum class eMaterialType : uint8
	{
		SPRITE,
		FULLSCREEN_PRESENT_LETTERBOX,
		MODEL3D,
		DEBUG_LINES2D,
		DEBUG_LINES3D,
		COUNT
	};

	enum class eShaderValueType : uint8
	{
		none, // Structs and other composite types
		boolean,
		int64,
		uint64,
		int32,
		uint32,
		int16,
		uint16,
		int8,
		uint8,
		float16,
		float32,
		float64,
	};

	// Each set corresponds to the update frequency of the data to the GPU
	// Global gets updated once per frame
	// Material is uploaded for a material domain, like sprites, debug lines and the like
	// Instance is uploaded on a per draw-call frequency
	enum eDecorationSets
	{
		GlobalDomain,
		MaterialTypeDomain,
		InstanceDomain,
		Count
	};

	enum class eDecorationType : uint8
	{
		UniformBuffer,
		ShaderStorageBuffer,
		SampledImage,
		PushConstant
	};
}