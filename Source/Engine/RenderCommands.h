#pragma once

#include "CommandsCommon.h"
#include "Interface\GameCommands.h"

namespace Hail
{
	constexpr uint32 LerpCommandFlagMask = (1u << 31);
	constexpr uint32 SizeRelativeToRenderTargetFlagMask = (1u << 31);
	constexpr uint32 IsSpriteFlagMask = (1u << 30);
	constexpr uint32 MaterialIndexIsValidMask = (MAX_UINT & (~(SizeRelativeToRenderTargetFlagMask | IsSpriteFlagMask)) >> 16);

	enum class eCommandType : uint8
	{
		Sprite,
		Text,
	};

	struct IndexMaterialIndexFlags
	{
		uint32 index : 16;
		uint32 materialIndex : 14;
		uint32 isSprite : 1;
		uint32 lerpCommand : 1;
	};

	union IndexMaterialIndexFlagsUnion
	{
		IndexMaterialIndexFlags bits;
		uint32 u;
	};

	// Pushed as is to the GPU, shared for sprites and texts
	struct RenderCommand2DBase
	{
		// Lerpable properties
		Transform2D m_transform;
		Color m_color;
		// unique index and Lerp Flag at last bit, second last bit (bit 31) is if it is a sprite command
		IndexMaterialIndexFlagsUnion m_index_materialIndex_flags; // 16bit, 14bit, 1bit, 1bit
		// Index in to this objects type specific data
		uint32 m_dataIndex{ MAX_UINT };
	};

	void LerpRenderCommand2DBase(RenderCommand2DBase& dst, const RenderCommand2DBase& readCommand, const RenderCommand2DBase& lastReadCommand, float t);

	struct RenderData_Sprite
	{
		glm::vec4 uvTR_BL = { 0.0, 0.0, 1.0, 1.0 }; // f2, f2
		glm::vec4 pivot_sizeMultiplier = { 0.5f, 0.5f, 1.f, 1.f }; // f2, f2
		glm::vec4 cutoutThreshold_padding; // f, f3
		glm::vec4 padding; // f, f3
	};
	
	struct RenderData_Text
	{
		StringLW text; // TODO: kolla hur jag sparar strängen, kanske ha en stor sträng pool bara för text commands, som är en circular buffer
		uint16 textSize;
	};

	struct RenderData_Mesh
	{
		Transform3D transform;
		Color color;
		uint32_t meshID = 0;
		uint32_t materialInstanceID = 0;
		uint16_t index = 0;
		bool bLerpCommand = true;
	};

	struct Batch2DInfo
	{
		uint16 m_instanceOffset{};
		uint16 m_numberOfInstances{};
		eCommandType m_type;
	};

	struct RenderCommandPool
	{
		Camera camera3D;
		VectorOnStack<Batch2DInfo, MAX_NUMBER_OF_SPRITES, false> m_batches;
		// Keeps the offset in to the batch list for each index, also is the counter for how many layers we will render
		VectorOnStack<uint16, MAX_NUMBER_OF_SPRITES, false> m_layersBatchOffset;
		// Gets uploaded as is to the GPU, can be lerped on either the GPU or the CPU
		VectorOnStack<RenderCommand2DBase, MAX_NUMBER_OF_2D_RENDER_COMMANDS, false> m_2DRenderCommands;
		VectorOnStack<RenderData_Sprite, MAX_NUMBER_OF_SPRITES, false> m_spriteData;
		VectorOnStack<RenderData_Text, MAX_NUMBER_OF_TEXT_COMMANDS, false> m_textData;
		VectorOnStack<RenderData_Mesh, 128, false> m_meshData;
		// TODO: define out for debug
		VectorOnStack<DebugLineCommand, MAX_NUMBER_OF_DEBUG_LINES / 2, false> m_debugLineCommands;
		VectorOnStack<DebugCircle, MAX_NUMBER_OF_DEBUG_CIRCLES, false> m_debugCircles;

	};

}

