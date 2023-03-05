#pragma once

#include "glm\mat4x4.hpp"
#include "glm\vec2.hpp"

namespace Hail
{
	enum class UNIFORM_BUFFERS : uint32_t
	{
		TUTORIAL,
		PER_FRAME_DATA,
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
		float totalTime;
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

	const uint32_t GetUniformBufferIndex(const UNIFORM_BUFFERS buffer);
	const uint32_t GetUniformBufferSize(const UNIFORM_BUFFERS buffer);



	struct Buffer
	{
		const uint32_t GetBufferSize();

		BUFFER_TYPE type{};
		uint32_t sizeInBytes = 0;
		uint32_t offset = 0;
	};
}

