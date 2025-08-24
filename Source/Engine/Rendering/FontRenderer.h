#pragma once
#include "Types.h"
#include "ResourceCommon.h"
#include "Utility\TTF_Parser.h"

namespace Hail
{
	class BufferObject;
	class ErrorManager;
	class MaterialPipeline;
	class Renderer;
	class ResourceManager;

	struct RenderCommandPool;

	struct RenderGlypghlet
	{
		glm::vec2 relativePos;
		float glyphPixelSize;
		uint32 glyphletColor;

		uint32 vertexOffset;
		uint32 indexOffset;
		uint32 numberOfTrianglesNumberOfVertices; // 16 bits Tri count | << 16 bits vert count
		uint32 commandBufferIndex; // 16 command buffer index | << 16 batch offset
	};

	struct TTF_FontStruct;

	class FontRenderer
	{
	public:
		~FontRenderer();
		explicit FontRenderer(Renderer* pRenderer, ResourceManager* pResourceManager);
		void Initialize(ErrorManager* pErrorManager);
		void Cleanup();

		void Prepare(const RenderCommandPool& poolOfCommands);
		void RenderBatch(uint32 numberOfInstances, uint32 batchOffset);

	private:


		MaterialPipeline* m_pFontPipeline;

		Renderer* m_pRenderer;
		ResourceManager* m_pResourceManager;

		BufferObject* m_pVertexBuffer;
		BufferObject* m_pIndexBuffer;
		BufferObject* m_pGlyphletBuffer;
		BufferObject* m_pTextCommandBuffer;
		BufferObject* m_pBatchOffsetBuffer;

		TTF_FontStruct m_fontData;

		GrowingArray<RenderGlypghlet> m_glyphletsToRender;
		// vec2 for command position (aka pivot), float for rotation 
		GrowingArray<glm::vec4> m_textCommandsToRender;
		GrowingArray<uint32> m_batchOffsetToInstanceStart;
		GrowingArray<uint32> m_batchNumberOfGlypsToRender;
	};

}

