#pragma once

#include "EngineConstants.h"

#include "glm\mat4x4.hpp"
#include "glm\vec2.hpp"

namespace Hail
{
	enum class BUFFERS : uint32_t
	{
		PER_FRAME_DATA,
		SPRITE_INSTANCE_BUFFER,
		TUTORIAL,
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

	struct SpriteInstanceData
	{
		glm::vec4 position_scale;
		glm::vec4 uvBL_TR;
		glm::vec4 color;
		glm::vec4 pivot_rotation_padding; //vec2, float, float
		glm::uvec4 textureSize_effectData_padding;
	};

	struct SpriteInstanceDataBuffer
	{
		SpriteInstanceData sprites[MAX_NUMBER_OF_SPRITES];
	};

	enum class SHADER_STORAGE_BUFFER_USAGE : uint32_t
	{
		READ,
		WRITE,
		READ_WRITE
	};

	enum class BUFFER_TYPE : uint32_t
	{
		VERTEX,
		INDEX,
		CONSTANT, // Uniform buffer in OpenGL
		SHADER_STORAGE, // Structured buffer in DX, read write on GPU
	};

	const uint32_t GetUniformBufferIndex(const BUFFERS buffer);
	const uint32_t GetUniformBufferSize(const BUFFERS buffer);



	struct BufferObject
	{
		const uint32_t GetBufferSize();

		BUFFER_TYPE type{};
		uint32_t sizeInBytes = 0;
		uint32_t offset = 0;
	};
}

