#pragma once
#include "Containers\GrowingArray\GrowingArray.h"

namespace Hail
{
	struct GlyphTri
	{
		uint32 indices[3];
		uint32 bIsConcave;
	};

	struct Glyph
	{
		bool m_bIsSimpleGlyph; // uint32, 1st bit,is compound, next 7 bits is a count of how many glyphs, 24 last bits are start index of glyph

		uint32 m_vertexOffset;  // if compound, compoundStartOffset in compound list
		uint32 m_indexOffset; // if compound, numberOfCompounds
		uint32 m_numberOfTrianglesNumberOfVertices; // 16 bits Tri count | << 16 bits vert count

		uint16 m_advanceWidth;
		int16 m_leftSideBearing;
		glm::ivec2 m_minExtent;
		glm::ivec2 m_maxExtent;
	};

	struct TTF_FontStruct
	{
		glm::ivec2 minGlyphBB;
		glm::ivec2 maxGlyphBB;
		glm::ivec2 m_glyphExtents;

		struct GlyphData
		{
			GrowingArray<Glyph> m_glyphs;
			GrowingArray<Glyph> m_compoundGlyphs;
			GrowingArray<glm::vec2> m_verts;
			GrowingArray<GlyphTri> m_triangles;
			uint16 m_maxNumbOfVerticesForGlyphs;
			uint16 m_maxNumbOfPrimitivesForGlyphs;
		} m_glyphData;

		GrowingArray<glm::vec4> m_renderVerts;
		GrowingArray<uint16> m_uniCodeToGlyphID;
	};

	// This function works, but is really unoptimized, but not important to fix now. Especially if this is done on a different thread.
	TTF_FontStruct TTF_ParseFontFile(const char* aFileToParse);
}