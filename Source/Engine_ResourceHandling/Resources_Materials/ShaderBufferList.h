#pragma once

namespace Hail
{
	// TODO make some kind of macro to define these and their buffer sizes, as they are defined by the engine and are fixed size
	// Set 0 
	enum class eGlobalUniformBuffers
	{
		frameData,
		viewData,
		count
	};
	enum class eGlobalStructuredBuffers
	{
		count
	};
	// Set 1
	enum class eMaterialUniformBuffers
	{
		vulkanTutorialUniformBUffer,
		count
	};
	enum class eMaterialBuffers
	{
		spriteInstanceBuffer,
		lineInstanceBuffer,
		count
	};
	// Set 2
	enum class eInstanceUniformBuffer
	{
		modelViewProjectionData, // temp buffer for now
		count
	};
	enum class eInstanceBuffer
	{

	};
}