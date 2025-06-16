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
		FULLSCREEN_PRESENT_LETTERBOX, // TODO: Move to Custom material type
		MODEL3D,
		CUSTOM, // Used for anything that is not batched unlike standardized sprite and mesh rendering.
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
		int16norm,
		uint16,
		uint16norm,
		int8,
		int8norm,
		uint8,
		uint8norm,
		float16,
		float32,
		float64,
		composite // used when shader values are for example 11f with 10f for an rgb value of 32bits
	};

	enum class eShaderAccessQualifier : uint8
	{
		ReadOnly,
		WriteOnly,
		ReadWrite
	};

	// Each set corresponds to the update frequency of the data to the GPU
	// Global gets updated once per frame and are owned by the engine
	// Material is uploaded for a material domain, like sprites, debug lines and custom pipelines.
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
		Image,
		Sampler,
		PushConstant,
		Count
	};


}