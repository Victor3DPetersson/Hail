#pragma once
#include "Types.h"
#include "ResourceCommon.h"
#include "Utility\TTF_Parser.h"

namespace Hail
{
	class MaterialPipeline;
	class Renderer;
	class ResourceManager;
	class BufferObject;
	struct TTF_FontStruct;

	class FontRenderer
	{
	public:
		explicit FontRenderer(Renderer* pRenderer, ResourceManager* pResourceManager);
		bool Initialize();

		void Render();

	private:

		MaterialPipeline* m_pFontPipeline;

		Renderer* m_pRenderer;
		ResourceManager* m_pResourceManager;

		BufferObject* m_pVertexBuffer;
		BufferObject* m_pIndexBuffer;
		BufferObject* m_pGlyphletBuffer;

		TTF_FontStruct m_fontData;
	};

}

