#pragma once

#include "EngineConstants.h"

#include "glm\mat4x4.hpp"
#include "glm\vec2.hpp"

namespace Hail
{
	enum class BUFFERS : uint32
	{
		PER_FRAME_DATA = 0u,
		SPRITE_INSTANCE_BUFFER = 1u,
		TUTORIAL = 2u,
		DEBUG_LINE_INSTANCE_BUFFER = 3u,
		PER_CAMERA_DATA = 4u,
		COUNT
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

	enum class SHADER_STORAGE_BUFFER_USAGE : uint32
	{
		READ,
		WRITE,
		READ_WRITE
	};

	enum class BUFFER_TYPE : uint32
	{
		VERTEX,
		INDEX,
		CONSTANT, // Uniform buffer in OpenGL
		SHADER_STORAGE, // Structured buffer in DX, read write on GPU
	};

	[[nodiscard]] const uint32_t GetUniformBufferIndex(const BUFFERS buffer);
	[[nodiscard]] const uint32_t GetUniformBufferSize(const BUFFERS buffer);



	struct BufferObject
	{
		const uint32 GetBufferSize();

		BUFFER_TYPE type{};
		uint32_t sizeInBytes = 0;
		uint32_t offset = 0;
	};
}

