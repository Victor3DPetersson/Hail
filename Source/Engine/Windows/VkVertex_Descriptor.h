#pragma once

#include "Resources\Vertices.h"
#include "Containers\GrowingArray\GrowingArray.h"

#include "vulkan\vulkan.h"

namespace Hail
{
	static inline VkVertexInputBindingDescription GetBindingDescription(VERTEX_TYPES vertexType)
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		switch (vertexType)
		{
		case Hail::VERTEX_TYPES::SPRITE:
			bindingDescription.stride = sizeof(VertexSprite);
			break;
		case Hail::VERTEX_TYPES::MODEL:
			bindingDescription.stride = sizeof(VertexModel);
			break;
		case Hail::VERTEX_TYPES::ANIMATION:
			bindingDescription.stride = sizeof(VertexAnimation);
			break;
		case Hail::VERTEX_TYPES::PARTICLE:
			break;
		default:
			break;
		}
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static inline GrowingArray<VkVertexInputAttributeDescription> GetAttributeDescriptions(VERTEX_TYPES vertexType)
	{
		switch (vertexType)
		{
		case Hail::VERTEX_TYPES::SPRITE:
		{
			GrowingArray<VkVertexInputAttributeDescription> attributeDescriptions{};
			attributeDescriptions.InitAndFill(3);
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(VertexSprite, pos);
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(VertexSprite, texCoord1);
			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(VertexSprite, color);
			return attributeDescriptions;
		}

		break;
		case Hail::VERTEX_TYPES::MODEL:
		{
			GrowingArray<VkVertexInputAttributeDescription> attributeDescriptions{};
			attributeDescriptions.InitAndFill(7);
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(VertexModel, pos);
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(VertexModel, norm);
			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(VertexModel, tangent);
			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(VertexModel, biTangent);
			attributeDescriptions[4].binding = 0;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[4].offset = offsetof(VertexModel, color);
			attributeDescriptions[5].binding = 0;
			attributeDescriptions[5].location = 5;
			attributeDescriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[5].offset = offsetof(VertexModel, texCoord1);
			attributeDescriptions[6].binding = 0;
			attributeDescriptions[6].location = 6;
			attributeDescriptions[6].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[6].offset = offsetof(VertexModel, texCoord2);
			return attributeDescriptions;
		}
		break;
		case Hail::VERTEX_TYPES::ANIMATION:
		{
			GrowingArray<VkVertexInputAttributeDescription> attributeDescriptions{};
			attributeDescriptions.InitAndFill(9);
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(VertexAnimation, pos);
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(VertexAnimation, norm);
			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(VertexAnimation, tangent);
			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(VertexAnimation, biTangent);
			attributeDescriptions[4].binding = 0;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[4].offset = offsetof(VertexAnimation, color);
			attributeDescriptions[5].binding = 0;
			attributeDescriptions[5].location = 5;
			attributeDescriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[5].offset = offsetof(VertexAnimation, texCoord1);
			attributeDescriptions[6].binding = 0;
			attributeDescriptions[6].location = 6;
			attributeDescriptions[6].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[6].offset = offsetof(VertexAnimation, texCoord2);
			attributeDescriptions[7].binding = 0;
			attributeDescriptions[7].location = 7;
			attributeDescriptions[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[7].offset = offsetof(VertexAnimation, weigths);
			attributeDescriptions[8].binding = 0;
			attributeDescriptions[8].location = 8;
			attributeDescriptions[8].format = VK_FORMAT_R32G32B32A32_UINT;
			attributeDescriptions[8].offset = offsetof(VertexAnimation, indices);
			return attributeDescriptions;
		}

		break;
		case Hail::VERTEX_TYPES::PARTICLE:
		{
			GrowingArray<VkVertexInputAttributeDescription> attributeDescriptions{};
			attributeDescriptions.InitAndFill(0);

			return attributeDescriptions;
		}

		break;
		default:
			break;
		}
	}
}