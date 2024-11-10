#pragma once

namespace Hail
{
	// TODO flytta ut s� att det endast �r globala buffrar och texturer som h�nger h�r, inte pass specific data som tex sprite buffer osv.
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