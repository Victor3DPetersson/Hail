#pragma once

#include "EngineConstants.h"
#include "Resources\MaterialResources.h"

#include "glm\mat4x4.hpp"
#include "glm\vec2.hpp"

namespace Hail
{
	class RenderingDevice;

	enum class eShaderBufferUsage : uint32
	{
		READ,
		WRITE,
		READ_WRITE
	};

	enum class eBufferType : uint32
	{
		vertex,
		index,
		uniform,
		structured, 
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
	};

	class BufferObject
	{
	public:
		virtual bool Init(RenderingDevice* device, BufferProperties properties) = 0;
		virtual void CleanupResource(RenderingDevice* device) = 0;

		const BufferProperties& GetProperties() { return m_properties; }
		const uint32 GetBufferSize() const;
		const uint32 GetID() const { return m_id; }
	protected:
		BufferProperties m_properties;
		uint32 m_id = UINT_MAX;
		static uint32 g_idCounter;
	};

}

