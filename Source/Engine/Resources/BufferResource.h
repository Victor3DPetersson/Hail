#pragma once

#include "EngineConstants.h"
#include "Resources\MaterialResources.h"

#include "glm\mat4x4.hpp"
#include "glm\vec2.hpp"

namespace Hail
{
	class RenderingDevice;

	enum class eShaderBufferUsage : uint8
	{
		Read,
		Write, // Does shaders write to this buffer
		ReadWrite, // RW buffer
		Undefined
	};

	enum class eShaderBufferDomain : uint8
	{
		GpuOnly,
		CpuOnly,
		GpuToCpu, // transfer
		CpuToGpu, // transfer
		Undefined
	};

	// For GPU only memory this can be left undefined, otherwise this must be specified
	enum class eShaderBufferUpdateFrequency : uint8
	{
		PerFrame, 
		Sporadic, // If this can be updated sometimes, 
		Once, // Will only update and send its data on creation, useful for uploading of large chunks of data, like font data
		Never,
		Undefined // 
	};

	enum class eBufferType : uint32
	{
		vertex,
		index,
		uniform,
		structured, 
		staging
	};

	struct TutorialUniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct PerFrameUniformBuffer
	{
		glm::uvec2 mainRenderResolution;
		glm::uvec2 mainWindowResolution;
		glm::vec2 totalTime_horizonLevel;
	};

	struct PerCameraUniformBuffer {
		glm::mat4 view;
		glm::mat4 proj;
	};

	// Structured Buffers

	struct SpriteInstanceData
	{
		glm::vec4 position_scale;
		glm::vec4 uvTR_BL;
		glm::vec4 color;
		glm::vec4 pivot_rotation_padding; //vec2, float, float
		glm::vec4 sizeMultiplier_effectData_cutoutThreshold_padding;
	};

	struct DebugLineData
	{
		glm::vec4 posAndIs2D;
		glm::vec4 color;
	};

	struct BufferProperties
	{
		eBufferType type;
		uint32 elementByteSize = 0;
		uint32 offset = 0;
		uint32 numberOfElements = 0;
		eShaderBufferUsage usage = eShaderBufferUsage::Undefined;
		eShaderBufferDomain domain = eShaderBufferDomain::Undefined;
		eShaderBufferUpdateFrequency updateFrequency = eShaderBufferUpdateFrequency::Undefined;
	};

	class BufferObject
	{
	public:
		bool Init(RenderingDevice* device, BufferProperties properties);
		virtual void CleanupResource(RenderingDevice* device) = 0;

		const BufferProperties& GetProperties() { return m_properties; }
		bool UsesFrameInFlight() const { return m_bUsesFramesInFlight; }
		virtual bool UsesPersistentMapping(RenderingDevice* device, uint32 frameInFlight) = 0; // 
		const uint32 GetBufferSize() const;
		const uint32 GetID() const { return m_id; }
	protected:

		virtual bool InternalInit(RenderingDevice* device) = 0;

		BufferProperties m_properties;
		uint32 m_id = UINT_MAX;
		static uint32 g_idCounter;
		bool m_bUsesFramesInFlight = false;
	};

}

