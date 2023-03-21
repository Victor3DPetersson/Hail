#pragma once

#include "glm\vec2.hpp"
#include "glm\vec3.hpp"
#include "glm\vec4.hpp"

//TEMP for tutorial
#include "vulkan\vulkan.h"
#include <array>

namespace Hail
{
	enum class VERTEX_TYPES : uint32_t
	{
		SPRITE,
		MODEL,
		ANIMATION,
		PARTICLE
	};

	struct VertexSprite
	{
		uint32_t index;
	};

	struct VertexModel
	{
		glm::vec3 pos = { 0.0, 0.0, 0.0 };
		glm::vec3 norm = { 0.0, 0.0, 0.0 };
		glm::vec3 tangent = { 0.0, 0.0, 0.0 };
		glm::vec3 biTangent = { 0.0, 0.0, 0.0 };
		glm::vec4 color = { 1.0, 1.0, 1.0, 1.0 };
		glm::vec2 texCoord1 = { 0.0, 0.0 };
		glm::vec2 texCoord2 = { 0.0, 0.0 };
	};

	struct VertexAnimation
	{
		glm::vec3 pos = { 0.0, 0.0, 0.0 };
		glm::vec3 norm = { 0.0, 0.0, 0.0 };
		glm::vec3 tangent = { 0.0, 0.0, 0.0 };
		glm::vec3 biTangent = { 0.0, 0.0, 0.0 };
		glm::vec4 color = { 1.0, 1.0, 1.0, 1.0 };
		glm::vec2 texCoord1 = { 0.0, 0.0 };
		glm::vec2 texCoord2 = { 0.0, 0.0 };
		glm::vec4 weigths = { 0.0, 0.0, 0.0, 0.0 };
		glm::u32vec4 indices = { 0, 0, 0, 0 };
	};
}


//struct TutorialVertex
//{
//	glm::vec3 pos;
//	glm::vec3 color;
//	glm::vec2 texCoord;
//
//	static VkVertexInputBindingDescription getBindingDescription() {
//		VkVertexInputBindingDescription bindingDescription{};
//		bindingDescription.binding = 0;
//		bindingDescription.stride = sizeof(TutorialVertex);
//		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//		return bindingDescription;
//	}
//
//
//	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
//		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
//		attributeDescriptions[0].binding = 0;
//		attributeDescriptions[0].location = 0;
//		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//		attributeDescriptions[0].offset = offsetof(TutorialVertex, pos);
//		attributeDescriptions[1].binding = 0;
//		attributeDescriptions[1].location = 1;
//		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//		attributeDescriptions[1].offset = offsetof(TutorialVertex, color);
//		attributeDescriptions[2].binding = 0;
//		attributeDescriptions[2].location = 2;
//		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
//		attributeDescriptions[2].offset = offsetof(TutorialVertex, texCoord);
//		return attributeDescriptions;
//	}
//};