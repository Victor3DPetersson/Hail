#pragma once
#include "Types.h"
#include "ResourceCommon.h"
#include "Utility\TTF_Parser.h"

namespace Hail
{
	class BufferObject;
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
		uint32 commandBufferIndex;
	};

	struct TTF_FontStruct;


	class FontRenderer
	{
	public:
		~FontRenderer();
		explicit FontRenderer(Renderer* pRenderer, ResourceManager* pResourceManager);
		bool Initialize();
		void Cleanup();

		void Prepare(const RenderCommandPool& poolOfCommands);
		void Render();

	private:


		MaterialPipeline* m_pFontPipeline;

		Renderer* m_pRenderer;
		ResourceManager* m_pResourceManager;

		BufferObject* m_pVertexBuffer;
		BufferObject* m_pIndexBuffer;
		BufferObject* m_pGlyphletBuffer;
		BufferObject* m_pTextCommandBuffer;

		TTF_FontStruct m_fontData;

		GrowingArray<RenderGlypghlet> m_glyphletsToRender;
		// vec2 for command position (aka pivot), float for rotation 
		GrowingArray<glm::vec4> m_textCommandsToRender;
	};

}

