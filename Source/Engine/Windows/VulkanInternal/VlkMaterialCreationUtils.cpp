#include "Engine_PCH.h"
#include "VlkMaterialCreationUtils.h"
#include "Resources\MaterialResources.h"
using namespace Hail;

void LocalCreateBlendMode(const Material& material, VkPipelineColorBlendAttachmentState& colorBLendAttachmentOut)
{
	switch (material.m_blendMode)
	{
	case Hail::eBlendMode::None:
	case Hail::eBlendMode::Cutout:
		colorBLendAttachmentOut.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBLendAttachmentOut.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBLendAttachmentOut.colorBlendOp = VK_BLEND_OP_ADD;
		colorBLendAttachmentOut.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBLendAttachmentOut.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBLendAttachmentOut.alphaBlendOp = VK_BLEND_OP_ADD;
		break;
	case Hail::eBlendMode::Translucent:
		colorBLendAttachmentOut.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBLendAttachmentOut.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBLendAttachmentOut.colorBlendOp = VK_BLEND_OP_ADD;
		colorBLendAttachmentOut.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBLendAttachmentOut.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBLendAttachmentOut.alphaBlendOp = VK_BLEND_OP_ADD;
		break;
	case Hail::eBlendMode::Additive:
		colorBLendAttachmentOut.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBLendAttachmentOut.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBLendAttachmentOut.colorBlendOp = VK_BLEND_OP_ADD;
		colorBLendAttachmentOut.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBLendAttachmentOut.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBLendAttachmentOut.alphaBlendOp = VK_BLEND_OP_ADD;
		break;
	default:
		break;
	}
}

VkPipelineColorBlendAttachmentState Hail::CreateColorBlendAttachment(const Material& material)
{
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = (material.m_blendMode == eBlendMode::Translucent || material.m_blendMode == eBlendMode::Additive) ? VK_TRUE : VK_FALSE;
	LocalCreateBlendMode(material, colorBlendAttachment);
	return colorBlendAttachment;
}
