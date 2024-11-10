#include "Shared_PCH.h"
#include "TTF_Parser.h"

#include "InOutStream.h"
#include "MathUtils.h"

#include "CDT\CDT.h"

#include "glm\geometric.hpp"
using namespace Hail;

namespace
{
	bool CheckIsValidTriangle(glm::vec2 a, glm::vec2 b, glm::vec2 c)
	{
		float d1 = a.length();
		float d2 = b.length();
		float d3 = c.length();

		// Length of sides must be positive  
		// and sum of any two sides  
		// must be smaller than third side.  
		if (d1 < 0 || d2 < 0 || d3 < 0 ||
			(d1 + d2 <= d3) || d1 + d3 <= d2 ||
			d2 + d3 <= d1)
		{
			return false;
		}
		float s = (d1 + d2 + d3) / 2;
		float area = sqrt(s * (s - d1) *
			(s - d2) * (s - d3));

		return area != 0.0;
	}

	glm::vec3 Barycentric(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 p)
	{
		glm::vec2 v0 = b - a, v1 = c - a, v2 = p - a;
		
		float d00 = glm::dot(v0, v0);
		float d01 = glm::dot(v0, v1);
		float d11 = glm::dot(v1, v1);
		float d20 = glm::dot(v2, v0);
		float d21 = glm::dot(v2, v1);
		float denom = d00 * d11 - d01 * d01;
		if (denom == 0.0)
			return glm::vec3(0);
		glm::vec3 bary;
		bary.y = (d11 * d20 - d01 * d21) / denom;
		bary.z = (d00 * d21 - d01 * d20) / denom;
		bary.x = 1.0f - bary.y - bary.z;

		return bary;
	}

	glm::vec2 computeUV(const glm::vec3 bary)
	{
		// The three control points a, c, b of the canonical Bézier curve:
		glm::vec2 a = glm::vec2(0.0f, 0.0f);
		glm::vec2 c = glm::vec2(0.5f, 0.0f);
		glm::vec2 b = glm::vec2(1.0f, 1.0f);
		// Explicitly carry out the interpolation using barycentrics.
		return bary.x * a + bary.y * c + bary.z * b;
	}

	float sign(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
	{
		return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
	}

	bool PointInTriangle(glm::vec2 pt, glm::vec2 v1, glm::vec2 v2, glm::vec2 v3)
	{
		float d1, d2, d3;
		bool has_neg, has_pos;

		d1 = sign(pt, v1, v2);
		d2 = sign(pt, v2, v3);
		d3 = sign(pt, v3, v1);

		has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
		has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

		return !(has_neg && has_pos);
	}

	struct FontDirectory
	{
		uint32 scalerType; //	A tag to indicate the OFA scaler to be used to rasterize this font; see the note on the scaler type below for more information.
		uint16 numTables; // 	number of tables
		uint16 searchRange; // (maximum power of 2 <= numTables) * 16
		uint16 entrySelector; // 	log2(maximum power of 2 <= numTables)
		uint16 rangeShift; // 	numTables * 16 - searchRange
	};

	struct Table
	{
		struct Tag
		{
			union {
				uint32 u32;
				uint8 c[4];
			}un;
		};
		Tag tag; // 	4 - byte identifier
		uint32 checkSum; // 	checksum for this table
		uint32 offset; // 	offset from beginning of sfnt
		uint32 length; // 	length of this table in byte(actual length not padded length)
	};


	struct GlyphDescription
	{
		int16 numberOfContours;// If the number of contours is positive or zero, it is a single glyph;
		// If the number of contours less than zero, the glyph is compound
		int16 xMin; // Minimum x for coordinate data
		int16 yMin; // Minimum y for coordinate data
		int16 xMax; // Maximum x for coordinate data
		int16 yMax; // Maximum y for coordinate data
	};

	struct HeadTable
	{
		int32 version;
		int32 fontRevision;

		uint32 checkSumAdjustment;
		uint32 magicNumber;

		uint16 flags;
		uint16 unitsPerEm;

		int64 created;
		int64 modified;

		int16 xMin;
		int16 yMin;
		int16 xMax;
		int16 yMax;

		uint16 macStyle;
		uint16 lowestRecPPEM;
		int16 fontDirectionHint;
		int16 indexToLocFormat;
		int16 glyphDataFormat;
	};

	int8 ReadInt8(char* pFontMemory, uint32 offset)
	{
		int16 value = 0;
		memcpy(&value, pFontMemory + offset, sizeof(int8));
		return value;
	}
	int8 ReadInt8Move(char* pFontMemory, uint32& offset)
	{
		int8 returnValue = ReadInt8(pFontMemory, offset);
		offset += sizeof(int8);
		return returnValue;
	}

	uint8 ReadUint8(char* pFontMemory, uint32 offset)
	{
		uint8 value = 0;
		memcpy(&value, pFontMemory + offset, sizeof(uint8));
		return value;
	}
	uint8 ReadUint8Move(char* pFontMemory, uint32& offset)
	{
		uint8 returnValue = ReadUint8(pFontMemory, offset);
		offset += sizeof(uint8);
		return returnValue;
	}

	int16 ReadInt16(char* pFontMemory, uint32 offset)
	{
		int16 value = 0;
		memcpy(&value, pFontMemory + offset, sizeof(int16));
		if (!IsSystemBigEndian())
			SwapEndian(value);
		return value;
	}
	int16 ReadInt16Move(char* pFontMemory, uint32& offset)
	{
		int16 returnValue = ReadInt16(pFontMemory, offset);
		offset += sizeof(int16);
		return returnValue;
	}

	uint16 ReadUint16(char* pFontMemory, uint32 offset)
	{
		uint16 value = 0;
		memcpy(&value, pFontMemory + offset, sizeof(uint16));
		if (!IsSystemBigEndian())
			SwapEndian(value);
		return value;
	}
	uint16 ReadUint16Move(char* pFontMemory, uint32& offset)
	{
		uint16 returnValue = ReadUint16(pFontMemory, offset);
		offset += sizeof(uint16);
		return returnValue;
	}

	int32 ReadInt32(char* pFontMemory, uint32 offset)
	{
		int32 value = 0;
		memcpy(&value, pFontMemory + offset, sizeof(int32));
		if (!IsSystemBigEndian())
			SwapEndian(value);
		return value;
	}
	int32 ReadInt32Move(char* pFontMemory, uint32& offset)
	{
		int32 returnValue = ReadInt32(pFontMemory, offset);
		offset += sizeof(int32);
		return returnValue;
	}

	uint32 ReadUint32(char* pFontMemory, uint32 offset)
	{
		uint32 value = 0;
		memcpy(&value, pFontMemory + offset, sizeof(uint32));
		if (!IsSystemBigEndian())
			SwapEndian(value);
		return value;
	}
	uint32 ReadUint32Move(char* pFontMemory, uint32& offset)
	{
		uint32 returnValue = ReadUint32(pFontMemory, offset);
		offset += sizeof(uint32);
		return returnValue;
	}

	struct HHEATable
	{
		int32 version;// 	0x00010000 (1.0)
		int16 ascent;// Distance from baseline of highest ascender
		int16 descent; //Distance from baseline of lowest descender
		int16 lineGap;//  typographic line gap
		uint16 advanceWidthMax;//must be consistent with horizontal metrics
		int16 minLeftSideBearing;//must be consistent with horizontal metrics
		int16 minRightSideBearing;//must be consistent with horizontal metrics
		int16 xMaxExtent;//max(lsb + (xMax - xMin))
		int16 caretSlopeRise;//used to calculate the slope of the caret(rise / run) set to 1 for vertical caret
		int16 caretSlopeRun;//0 for vertical
		int16 caretOffset;//set value to 0 for non - slanted fonts
		int16 reserved1;//set value to 0
		int16 reserved2;//set value to 0
		int16 reserved3;//set value to 0
		int16 reserved4;//set value to 0
		int16 metricDataFormat;//0 for current format
		uint16 numOfLongHorMetrics;//number of advance widths in metrics table
	};

	struct UnicodeToGlyphID
	{
		uint16 unicode;
		uint16 glyphID;
	};

	struct GlyphCoord
	{
		int x;
		int y;
		bool bOnLine;
	};

	struct ConstructionTri
	{
		glm::vec2 points[3];
		uint32 indices[3];
		uint8 bIsConcave;
	};

	struct Edge
	{
		std::pair<std::size_t, std::size_t> vertices;
	};

	GrowingArray<int> ReadSimpleGlyphCoordinates(char* pFontMemory, Glyph& glyph, const GrowingArray<uint8>& allFlags, uint32& currentFontMemoryOffset, bool readingX)
	{
		const int offsetSizeFlagBit = readingX ? 1 : 2;
		const int offsetSignOrSkipBit = readingX ? 4 : 5;
		GrowingArray<int> coordinates(allFlags.Size());
		int previousCoord = 0;

		for (int i = 0; i < allFlags.Size(); i++)
		{
			int& currentCoord = coordinates.Add(previousCoord);
			uint8 flag = allFlags[i];

			if (IsBitSet(flag, offsetSizeFlagBit))
			{
				uint8 offset = ReadUint8(pFontMemory, currentFontMemoryOffset++);
				int sign = IsBitSet(flag, offsetSignOrSkipBit) ? 1 : -1;
				currentCoord += offset * sign;
			} 
			else if (!IsBitSet(flag, offsetSignOrSkipBit))
			{
				int16 coord = ReadInt16(pFontMemory, currentFontMemoryOffset);
				currentFontMemoryOffset += sizeof(coord);
				currentCoord += (coord);
			}
			previousCoord = currentCoord;
		}
		return coordinates;
	}

	struct GlyphTransform
	{
		int32 a = 1, b = 0, c = 0, d = 1, e = 0, f = 0, m = 0, n = 0;
		int32 destPointIndex = -1, srcPointIndex = -1;
	};

	Glyph ParseGlyph(char* pFontMemory, const GrowingArray<uint32>& glyphOffsets, uint32 glyphOffset, GrowingArray<Glyph>& outCompoundGlyphs, GrowingArray<glm::vec2>& outVerts, GrowingArray<GlyphTri>& outTriangles, uint16& outMaxNumberOfVertices, uint16& outMaxNumbOfPrimitives, GlyphTransform transform);

	Glyph ParseSimpleGlyph(char* pFontMemory, const GlyphDescription& description, uint32 currentOffset, GrowingArray<glm::vec2>& outVerts, GrowingArray<GlyphTri>& outTriangles, GlyphTransform transform)
	{
		//uint16 	endPtsOfContours[n];// 	Array of last points of each contour; n is the number of contours; array entries are point indices
		//uint16 	instructionLength;// 	Total number of bytes needed for instructions
		//uint8 	instructions[instructionLength]; // 	Array of instructions for this glyph
		//uint8 	flags[variable];// 	Array of flags
		//uint8 or int16 	xCoordinates[]; // 	Array of x - coordinates; the first is relative to(0, 0), others are relative to previous point
		//uint8 or int16 	yCoordinates[]; // 	Array of y - coordinates; the first is relative to(0, 0), others are relative to previous point

		Glyph glyph;

		GrowingArray<uint16> endPts;
		endPts.PrepareAndFill(description.numberOfContours);

		for (size_t i = 0; i < description.numberOfContours; i++)
		{
			endPts[i] = ReadUint16(pFontMemory, currentOffset + sizeof(uint16) * i);
		}
		currentOffset = currentOffset + sizeof(uint16) * description.numberOfContours;

		uint16 instructionLength = ReadUint16(pFontMemory, currentOffset);
		currentOffset += sizeof(uint16);

		GrowingArray<uint8> instructions;
		instructions.PrepareAndFill(instructionLength);
		for (size_t i = 0; i < instructionLength; i++)
		{
			instructions[i] = ReadUint8(pFontMemory, currentOffset++);
		}

		const uint32 numberOfPts = endPts.Empty() ? 1 : endPts.GetLast() + 1;
		GrowingArray<uint8> flags(numberOfPts);
		for (size_t i = 0; i < numberOfPts; i++)
		{
			uint8 flag = ReadUint8(pFontMemory, currentOffset++);
			flags.Add(flag);
			if (IsBitSet(flag, 3))
			{
				uint8 numberOfFlags = ReadUint8(pFontMemory, currentOffset++);
				i += numberOfFlags;
				while (numberOfFlags--) 
				{
					flags.Add(flag);
				};
			}
		}

		GrowingArray<int> xCoords = ReadSimpleGlyphCoordinates(pFontMemory, glyph, flags, currentOffset, true);
		GrowingArray<int> yCoords = ReadSimpleGlyphCoordinates(pFontMemory, glyph, flags, currentOffset, false);

		GrowingArray<GlyphCoord> coords;
		coords.Prepare(flags.Size());

		const int onCurveBit = 0;
		uint16 startIndex = 0;
		GrowingArray<uint16> endPointAdjustments(endPts.Size());
		// Insert on the line points and adjust endpoint list.
		for (int iSpan = 0; iSpan < endPts.Size(); iSpan++)
		{
			const uint16 endPointSet = endPts[iSpan] + 1;

			for (int i = startIndex; i < endPointSet; i++)
			{
				const int iNextCoord = (i + 1) % (endPointSet);
				uint8 flag = flags[i];
				uint8 nextFlag = flags[iNextCoord];
				const bool bOnCurve = IsBitSet(flag, onCurveBit);
				const bool bNexPointIsOnCurve = IsBitSet(nextFlag, onCurveBit);

				const int xCoord = transform.a * xCoords[i] + transform.b * yCoords[i] + transform.e;
				const int yCoord = transform.c * xCoords[i] + transform.d * yCoords[i] + transform.f;

				const int nextXCoord =transform.a * xCoords[iNextCoord] + transform.b * yCoords[iNextCoord] + transform.e;
				const int nextYCoord =transform.c * xCoords[iNextCoord] + transform.d * yCoords[iNextCoord] + transform.f;

				{
					GlyphCoord coord;
					coord.bOnLine = bOnCurve;
					coord.x = xCoord;
					coord.y = yCoord;

					coords.Add(coord);
				}
				{
					if (!bOnCurve && !bNexPointIsOnCurve)
					{
						GlyphCoord inBetweenCoord;
						inBetweenCoord.bOnLine = true;
						inBetweenCoord.x = xCoord - ((xCoord - nextXCoord) / 2);
						inBetweenCoord.y = yCoord - ((yCoord - nextYCoord) / 2);
						coords.Add(inBetweenCoord);
					}
				}
			}
			endPointAdjustments.Add(coords.Size() - endPointSet);
			startIndex = endPointSet;
		}

		const uint16 numberOfNewlyAddedPoints = coords.Size() - flags.Size();

		if (numberOfNewlyAddedPoints)
		{
			for (int iSpan = 0; iSpan < endPts.Size(); iSpan++)
			{
				endPts[iSpan] += endPointAdjustments[iSpan];
			}
		}


		std::vector<CDT::V2d<float>> points;
		std::vector<CDT::Edge> edges;

		GrowingArray<GrowingArray<Edge>> spanOfEdges;

		for (int i = 0; i < coords.Size(); i++)
		{
			points.push_back({ (float)coords[i].x, (float)coords[i].y });
		}

		// Creating edges
		startIndex = 0;
		for (int iSpan = 0; iSpan < endPts.Size(); iSpan++)
		{
			GrowingArray<Edge>& spanEdges = spanOfEdges.Add();
			const uint16 adjustedEndPoint = (uint16)(endPts[iSpan] + 1);	

			for (int i = startIndex; i < adjustedEndPoint; i++)
			{
				const int nextCoordUnadjusted = (i + 1) % adjustedEndPoint;
				const int iNextCoord = nextCoordUnadjusted == 0 ? nextCoordUnadjusted + startIndex : nextCoordUnadjusted;
				Edge edge;
				edge.vertices.first = i;
				edge.vertices.second = iNextCoord;
				spanEdges.Add(edge);
				CDT::Edge cdtEdge = CDT::Edge(i, iNextCoord);
				edges.push_back(cdtEdge);
			}
			startIndex = adjustedEndPoint;
		}


		//Remove all tris that contain offCurve vertices, and remap edges.
		// Create mesh without any off-edges with these vectors
		std::vector<CDT::V2d<float>> pointsOnCurve;
		std::vector<CDT::Edge> edgesWithoutOffCurveVerts;

		// cached offedge information for final mesh
		GrowingArray<uint16> endPtsWithoutOffEdges;
		startIndex = 0;
		for (int iSpan = 0; iSpan < endPts.Size(); iSpan++)
		{
			const uint16 endPointSet = endPts[iSpan] + 1;

			for (int i = startIndex; i < endPointSet; i++)
			{
				if (coords[i].bOnLine)
				{
					pointsOnCurve.push_back({(float)coords[i].x, (float)coords[i].y});
				}
			}
			endPtsWithoutOffEdges.Add(pointsOnCurve.size() - 1);
			startIndex = endPointSet;
		}
		startIndex = 0;
		for (int iSpan = 0; iSpan < endPtsWithoutOffEdges.Size(); iSpan++)
		{
			const uint16 adjustedEndPoint = (uint16)(endPtsWithoutOffEdges[iSpan] + 1);

			for (int i = startIndex; i < adjustedEndPoint; i++)
			{
				const int nextCoordUnadjusted = (i + 1) % adjustedEndPoint;
				const int iNextCoord = nextCoordUnadjusted == 0 ? nextCoordUnadjusted + startIndex : nextCoordUnadjusted;
				CDT::Edge cdtEdge = CDT::Edge(i, iNextCoord);
				edgesWithoutOffCurveVerts.push_back(cdtEdge);
				Edge edge;
				edge.vertices.first = i;
				edge.vertices.second = iNextCoord;
			}
			startIndex = adjustedEndPoint;
		}

		CDT::DuplicatesInfo duplicatesInfo = CDT::RemoveDuplicatesAndRemapEdges<float>(pointsOnCurve, edgesWithoutOffCurveVerts);

		CDT::Triangulation<float> cdt;
		cdt.insertVertices(pointsOnCurve);
		cdt.conformToEdges(edgesWithoutOffCurveVerts);

		cdt.eraseOuterTrianglesAndHoles();

		GrowingArray<ConstructionTri> trianglesWithOffEdgeVerts;
		//Code to find triangles with off edge verts, all these triangles are triangles that will be treated specially in the final mesh
		for (int iSpan = 0; iSpan < spanOfEdges.Size(); iSpan++)
		{
			GrowingArray<Edge>& spanEdges = spanOfEdges[iSpan];

			bool bPreviousEdgeStartsOfLine = false;
			for (int iEdge = 0; iEdge < spanEdges.Size(); iEdge++)
			{
				int iNextEdgeIndex = iEdge + 1 == spanEdges.Size() ? 0 : iEdge + 1;

				// Check if the inbetween point is not on the line and if it is a quadratic bezier curve. 
				const bool firstEdgeContainsOffPoint = coords[spanEdges[iEdge].vertices.first].bOnLine && !coords[spanEdges[iEdge].vertices.second].bOnLine;
				const bool secondEdgeContainsOffPoint = !coords[spanEdges[iNextEdgeIndex].vertices.first].bOnLine && coords[spanEdges[iNextEdgeIndex].vertices.second].bOnLine;

				if (firstEdgeContainsOffPoint && secondEdgeContainsOffPoint)
				{
					GlyphCoord coord1 = coords[spanEdges[iEdge].vertices.first];
					GlyphCoord coord2 = coords[spanEdges[iEdge].vertices.second];
					GlyphCoord coord3 = coords[spanEdges[iNextEdgeIndex].vertices.second];

					//Remove edges that are on a straight line (probably a user error when creating the glyph).
					if ((coord1.x + ((coord3.x - coord1.x) / 2) == coord2.x && coord1.y + ((coord3.y - coord1.y) / 2) == coord2.y) || 
						!CheckIsValidTriangle(glm::vec2(coord1.x, coord1.y), glm::vec2(coord2.x, coord2.y), glm::vec2(coord3.x, coord3.y)))
					{
						continue;
					}

					if (coord1.x == coord2.x && coord1.x == coord3.x || coord1.y == coord2.y && coord1.y == coord3.y)
						continue;

					if (coord1.x == coord2.x && coord1.y == coord2.y || coord1.x == coord2.x && coord1.y == coord3.y || coord2.x == coord3.x && coord2.y == coord3.y)
						continue;

					glm::vec2 vertexCoords[3] = { glm::vec2(coord1.x, coord1.y), glm::vec2(coord2.x, coord2.y), glm::vec2(coord3.x, coord3.y) };

					ConstructionTri triangle;
					memcpy(triangle.points, vertexCoords, sizeof(glm::vec2) * 3);

					bool isInsideOfMesh = false;

					for (CDT::Triangle triangle : cdt.triangles)
					{
						if (PointInTriangle(vertexCoords[1], 
							{ cdt.vertices[triangle.vertices[0]].x, cdt.vertices[triangle.vertices[0]].y },
							{ cdt.vertices[triangle.vertices[1]].x, cdt.vertices[triangle.vertices[1]].y },
							{ cdt.vertices[triangle.vertices[2]].x, cdt.vertices[triangle.vertices[2]].y }))
						{
							isInsideOfMesh = true;
							break;
						}
					}
					triangle.indices[0] = spanEdges[iEdge].vertices.first;
					triangle.indices[1] = spanEdges[iEdge].vertices.second;
					triangle.indices[2] = spanEdges[iNextEdgeIndex].vertices.second;
					triangle.bIsConcave = !isInsideOfMesh;
					trianglesWithOffEdgeVerts.Add(triangle);
				}
			}
		}

		GrowingArray<GrowingArray<GlyphCoord>> finalCoords;

		// Loop over all coords to move the ones that are overlapping with convex triangles
		std::vector<CDT::V2d<float>> pointsInFinalTriangulation;
		startIndex = 0;
		for (int iSpan = 0; iSpan < endPts.Size(); iSpan++)
		{
			const uint16 endPointSet = endPts[iSpan] + 1;

			GrowingArray<GlyphCoord>& coordsToMakeFinalMeshOutOf = finalCoords.Add();

			for (int i = startIndex; i < endPointSet; i++)
			{
				GlyphCoord& coordToCheck = coords[i];
				bool addCoord = true;
				for (int iTriWithOffEdgeVert = 0; iTriWithOffEdgeVert < trianglesWithOffEdgeVerts.Size(); iTriWithOffEdgeVert++)
				{
					ConstructionTri& triToCheckAgainst = trianglesWithOffEdgeVerts[iTriWithOffEdgeVert];

					bool pointsIsPartOfTriangle = coordToCheck.x == (int)triToCheckAgainst.points[0].x && coordToCheck.y == (int)triToCheckAgainst.points[0].y ||
						coordToCheck.x == (int)triToCheckAgainst.points[1].x && coordToCheck.y == (int)triToCheckAgainst.points[1].y ||
						coordToCheck.x == (int)triToCheckAgainst.points[2].x && coordToCheck.y == (int)triToCheckAgainst.points[2].y;

					bool isAnOffEdgeVertAndConcaveControlVert = !coordToCheck.bOnLine && pointsIsPartOfTriangle && (coordToCheck.x == (int)triToCheckAgainst.points[1].x && coordToCheck.y == (int)triToCheckAgainst.points[1].y);
					
					addCoord &= !isAnOffEdgeVertAndConcaveControlVert;

					if (!triToCheckAgainst.bIsConcave && !pointsIsPartOfTriangle &&  
						PointInTriangle(glm::vec2(coordToCheck.x, coordToCheck.y), triToCheckAgainst.points[0], triToCheckAgainst.points[1], triToCheckAgainst.points[2]))
					{
						glm::vec3 baryCoords = Barycentric(triToCheckAgainst.points[0], triToCheckAgainst.points[1], triToCheckAgainst.points[2], glm::vec2(coordToCheck.x, coordToCheck.y));
						if (baryCoords == glm::vec3(0.0))
							continue;

						int closestBaryCoord = 0;
						float closestBaryValue = Math::Max(baryCoords.x, Math::Max(baryCoords.y, baryCoords.z));
						if (closestBaryValue == baryCoords.y)
							closestBaryCoord = 1;
						else if (closestBaryValue == baryCoords.z)
							closestBaryCoord = 2;
							
						glm::vec2 directionToClosestPoint = glm::vec2(coordToCheck.x, coordToCheck.y) - triToCheckAgainst.points[closestBaryCoord];
						float distanceToMove = glm::distance(glm::vec2(coordToCheck.x, coordToCheck.y), triToCheckAgainst.points[closestBaryCoord]);
						glm::vec2 vectorToMove = glm::normalize(directionToClosestPoint) * distanceToMove * -1.0f;
						coordToCheck.x += (int)vectorToMove.x;
						coordToCheck.y += (int)vectorToMove.y;
					}

					if (addCoord == false)
					{
						if (pointsIsPartOfTriangle && triToCheckAgainst.bIsConcave == 0 && !coordToCheck.bOnLine)
						{
							addCoord = true;
							break;
						}
					}
				}

				if (addCoord)
				{
					coordsToMakeFinalMeshOutOf.Add(coordToCheck);
					pointsInFinalTriangulation.push_back({(float)coordToCheck.x, (float)coordToCheck.y});
				}
			}
			startIndex = endPointSet;
		}

		// Create function for making final mesh. 
		GrowingArray<uint16> finalEndPoints;
		uint16 previousEndPoint = 0;
		for (size_t i = 0; i < finalCoords.Size(); i++)
		{
			finalEndPoints.Add(previousEndPoint + (finalCoords[i].Size() - 1) + i);
			previousEndPoint = previousEndPoint + (finalCoords[i].Size() - 1);
		}
		std::vector<CDT::Edge> edgesOfFinalTriangulation;
		startIndex = 0;
		for (int iSpan = 0; iSpan < finalEndPoints.Size(); iSpan++)
		{
			const uint16 adjustedEndPoint = (finalEndPoints[iSpan] + 1);
			for (int i = startIndex; i < adjustedEndPoint; i++)
			{
				const int nextCoordUnadjusted = (i + 1) % adjustedEndPoint;
				const int iNextCoord = nextCoordUnadjusted == 0 ? nextCoordUnadjusted + startIndex : nextCoordUnadjusted;
				CDT::Edge cdtEdge = CDT::Edge(i, iNextCoord);
				edgesOfFinalTriangulation.push_back(cdtEdge);
			}
			startIndex = adjustedEndPoint;
		}

		CDT::DuplicatesInfo finalDuplicatesInfo = CDT::RemoveDuplicatesAndRemapEdges<float>(pointsInFinalTriangulation, edgesOfFinalTriangulation);

		CDT::Triangulation<float> finalCdt;
		finalCdt.insertVertices(pointsInFinalTriangulation);
		finalCdt.conformToEdges(edgesOfFinalTriangulation);

		finalCdt.eraseOuterTrianglesAndHoles();

		//Move over all verts and add the ones from the concave triangles
		const uint32 verticesStartIndex = outVerts.Size();
		for (int iVert = 0; iVert < finalCdt.vertices.size(); iVert++)
		{
			outVerts.Add({ finalCdt.vertices[iVert].x, finalCdt.vertices[iVert].y });
		}

		for (int iTri = 0; iTri < finalCdt.triangles.size(); iTri++)
		{
			CDT::Triangle& triangle = finalCdt.triangles[iTri];
			GlyphTri triToAdd;
			triToAdd.bIsConcave = 2;
			memcpy(triToAdd.indices, triangle.vertices.data(), sizeof(triangle.vertices[0]) * 3);
			for (uint32 t = 0; t < 3; t++)
			{
				triToAdd.indices[t] += verticesStartIndex;
			}
			outTriangles.Add(triToAdd);
		}

		// Adding the triangles with the control vertex
		const uint32 lastVertexIndex = outVerts.Size();
		for (int iTriWithOffEdgeVert = 0; iTriWithOffEdgeVert < trianglesWithOffEdgeVerts.Size(); iTriWithOffEdgeVert++)
		{
			ConstructionTri& triWithOffEdgeVert = trianglesWithOffEdgeVerts[iTriWithOffEdgeVert];
			GlyphTri& triToAdd = outTriangles.Add();
			triToAdd.bIsConcave = triWithOffEdgeVert.bIsConcave;
			uint32 vertsFound = 0;
			bool triIndicesFound[3] = { false, false, false };
			for (uint32 iVert = verticesStartIndex; iVert < lastVertexIndex; iVert++)
			{
				if (triWithOffEdgeVert.points[0] == outVerts[iVert])
				{
					triIndicesFound[0] = true;
					triToAdd.indices[0] = iVert;
					vertsFound++;
				}
				else if (triWithOffEdgeVert.points[1] == outVerts[iVert])
				{
					triIndicesFound[1] = true;
					triToAdd.indices[1] = iVert;
					vertsFound++;
				}
				else if (triWithOffEdgeVert.points[2] == outVerts[iVert])
				{
					triIndicesFound[2] = true;
					triToAdd.indices[2] = iVert;
					vertsFound++;
				}

				if (vertsFound == 3)
					break;
			}
			// Add the missing vert to the vertexList
			if (vertsFound != 3)
			{
				for (uint32 t = 0; t < 3; t++)
				{
					if (!triIndicesFound[t])
					{
						vertsFound++;
						triToAdd.indices[t] = outVerts.Size();
						outVerts.Add(triWithOffEdgeVert.points[t]);
					}
				}
			}

			H_ASSERT(vertsFound == 3, "Incorrect number of vertices found");
			vertsFound = vertsFound;
		}

		return glyph;
	}

	Glyph ParseCompoundGlyph(char* pFontMemoryGlyfTableBase, char* pGlyfMemory, const GrowingArray<uint32>& glyphOffsets, GrowingArray<Glyph>& outCompoundGlyphs, GrowingArray<glm::vec2>& outVerts, GrowingArray<GlyphTri>& outTriangles, uint16& outMaxNumberOfVertices, uint16& outMaxNumbOfPrimitives)
	{
		Glyph compoundGlyph;

		compoundGlyph.m_bIsSimpleGlyph = 0;

		// Flag bits
		uint32 ARG_1_AND_2_ARE_WORDS = 1,
		ARGS_ARE_XY_VALUES = 2,
		ROUND_XY_TO_GRID = 4,
		WE_HAVE_A_SCALE = 8,
		// RESERVED = 16
		MORE_COMPONENTS_TO_READ = 32,
		WE_HAVE_AN_X_AND_Y_SCALE = 64,
		WE_HAVE_A_TWO_BY_TWO = 128,
		WE_HAVE_INSTRUCTIONS = 256,
		USE_MY_METRICS = 512,
		OVERLAP_COMPONENT = 1024;

		uint16 flags = MORE_COMPONENTS_TO_READ;

		uint32 numberOfCompounds = 0;
		uint32 compoundStartOffset = outCompoundGlyphs.Size();

		uint32 offset = 0;
		while (flags & MORE_COMPONENTS_TO_READ)
		{
			uint16 glyphIndex;

			char* m = pGlyfMemory;

			flags = ReadUint16Move(m, offset);
			glyphIndex = ReadUint16Move(m, offset);


			GlyphTransform transform;
			int32 arg1, arg2;
			if (flags & ARG_1_AND_2_ARE_WORDS)
			{
				arg1 = ReadInt16Move(m, offset);
				arg2 = ReadInt16Move(m, offset);
			}
			else
			{
				arg1 = ReadUint8Move(m, offset);
				arg2 = ReadUint8Move(m, offset);
			}

			if (flags & ARGS_ARE_XY_VALUES) {
				transform.e = arg1;
				transform.f = arg2;
			}
			else
			{
				transform.destPointIndex = arg1;
				transform.srcPointIndex = arg2;
			}

			if (flags & WE_HAVE_A_SCALE)
			{
				transform.a = ReadInt16Move(m, offset) / (1 << 14);
				transform.d = transform.a;
			}
			else if (flags & WE_HAVE_AN_X_AND_Y_SCALE)
			{
				transform.a = ReadInt16Move(m, offset) / (1 << 14);
				transform.d = ReadInt16Move(m, offset) / (1 << 14);
			}
			else if (flags & WE_HAVE_A_TWO_BY_TWO)
			{
				transform.a = ReadInt16Move(m, offset) / (1 << 14);
				transform.b = ReadInt16Move(m, offset) / (1 << 14);
				transform.c = ReadInt16Move(m, offset) / (1 << 14);
				transform.d = ReadInt16Move(m, offset) / (1 << 14);
			}

			Glyph glyph = ParseGlyph(pFontMemoryGlyfTableBase, glyphOffsets, glyphOffsets[glyphIndex], outCompoundGlyphs, outVerts, outTriangles, outMaxNumberOfVertices, outMaxNumbOfPrimitives, transform);

			outCompoundGlyphs.Add(glyph);
			numberOfCompounds++;
		}
		compoundGlyph.m_indexOffset = numberOfCompounds;
		compoundGlyph.m_vertexOffset = compoundStartOffset;

		return compoundGlyph;
	}

	Glyph ParseGlyph(char* pFontMemoryGlyfTableBase, const GrowingArray<uint32>& glyphOffsets, uint32 glyphOffset, GrowingArray<Glyph>& outCompoundGlyphs, GrowingArray<glm::vec2>& outVerts, GrowingArray<GlyphTri>& outTriangles, uint16& outMaxNumberOfVertices, uint16& outMaxNumbOfPrimitives, GlyphTransform transform)
	{
		GlyphDescription glyfDescription;
		memcpy(&glyfDescription, pFontMemoryGlyfTableBase + glyphOffset, sizeof(GlyphDescription));

		uint16 numberOfTriangles = 0;
		uint16 numberOfVerts = 0;
		if (!IsSystemBigEndian())
		{
			SwapEndian(glyfDescription.numberOfContours);
			SwapEndian(glyfDescription.xMax);
			SwapEndian(glyfDescription.xMin);
			SwapEndian(glyfDescription.yMax);
			SwapEndian(glyfDescription.yMin);
		}
		Glyph glyph;
		memset(&glyph, 0, sizeof(Glyph));

		const uint32 indexOffset = outTriangles.Size();
		const uint32 vertexOffset = outVerts.Size();

		if (glyfDescription.numberOfContours >= 0)
		{
			glyph = ParseSimpleGlyph(pFontMemoryGlyfTableBase, glyfDescription, glyphOffset + sizeof(GlyphDescription), outVerts, outTriangles, transform);
			numberOfTriangles = outTriangles.Size() - indexOffset;
			numberOfVerts = outVerts.Size() - vertexOffset;
			glyph.m_bIsSimpleGlyph = 1;
			glyph.m_numberOfTrianglesNumberOfVertices = numberOfTriangles | (numberOfVerts << 16);
			glyph.m_indexOffset = indexOffset;
			glyph.m_vertexOffset = vertexOffset;
		}
		else
		{
			glyph = ParseCompoundGlyph(pFontMemoryGlyfTableBase, pFontMemoryGlyfTableBase + (glyphOffset) + sizeof(GlyphDescription), glyphOffsets, outCompoundGlyphs, outVerts, outTriangles, outMaxNumberOfVertices, outMaxNumbOfPrimitives);
		}

		outMaxNumberOfVertices = Math::Max(outMaxNumberOfVertices, numberOfVerts);
		outMaxNumbOfPrimitives = Math::Max(outMaxNumbOfPrimitives, numberOfTriangles);

		glyph.m_minExtent = glm::ivec2(glyfDescription.xMin, glyfDescription.yMin);
		glyph.m_maxExtent = glm::ivec2(glyfDescription.xMax, glyfDescription.yMax);

		return glyph;
	}

	// Returns the max UnicodeValue that we find mapped to the subFormat, returns -1 if non is found on this table.
	int ReadCmapSubFormat(char* mem, GrowingArray<UnicodeToGlyphID>& outMappings)
	{
		char* m = mem;
		uint32 offset = 0;
		offset = 0;
		m = mem;

		int maxUniCodeValueInSubFormat = -1;

		uint16 format = ReadUint16Move(m, offset);
		if (format == 12)
		{
			uint16 reserved = ReadUint16Move(m, offset);
			uint32 subTableByteLengthIncludingHeader = ReadUint32Move(m, offset);
			uint32 languageCode = ReadUint32Move(m, offset);
			uint32 numGroups = ReadUint32Move(m, offset);

			for (int i = 0; i < numGroups; i++)
			{
				uint32 startCharCode = ReadUint32Move(m, offset);
				uint32 endCharCode = ReadUint32Move(m, offset);
				uint32 startGlyphIndex = ReadUint32Move(m, offset);

				uint32 numChars = endCharCode - startCharCode + 1;

				for (int iCharOffset = 0; iCharOffset < numChars; iCharOffset++)
				{
					//H_ASSERT(startCharCode + iCharOffset < MAX_UINT16, "Rework CodeToGlyphID size");
					uint16 charCode = (uint16)(startCharCode + iCharOffset);
					H_ASSERT(startGlyphIndex + iCharOffset < MAX_UINT16, "Rework CodeToGlyphID size");
					uint16 glyphIndex = (uint16)(startGlyphIndex + iCharOffset);

					auto searchCondition = [&](const UnicodeToGlyphID& a) {
						if (a.unicode == charCode)
							return true;
						return false;
					};

					maxUniCodeValueInSubFormat = Math::Max((int)charCode, maxUniCodeValueInSubFormat);

					if (outMappings.FindCustomCondition(searchCondition) == -1)
						outMappings.Add(UnicodeToGlyphID{ charCode, glyphIndex });
				}
			}

		}
		else if (format == 4)
		{
			offset += sizeof(uint16) * 2; // Skip length and language code

			uint16 segCount2X = ReadUint16Move(m, offset);
			uint16 segCount = segCount2X / 2;

			offset += sizeof(uint16) * 3; // Skip searchRange, engtrySelector and rangeShift

			GrowingArray<uint16> endCodes(segCount, 0);

			for (int i = 0; i < segCount; i++)
			{
				endCodes[i] = ReadUint16Move(m, offset);
			}

			offset += sizeof(uint16); // Skip reserved padding

			GrowingArray<uint16> startCodes(segCount, 0);

			for (int i = 0; i < segCount; i++)
			{
				startCodes[i] = ReadUint16Move(m, offset);
			}

			GrowingArray<uint16> idDeltas(segCount, 0);

			for (int i = 0; i < segCount; i++)
			{
				idDeltas[i] = ReadUint16Move(m, offset);
			}

			struct LocationOffset
			{
				uint16 bufferLocation;
				uint16 offset;
			};
			GrowingArray<LocationOffset> idRangeOffsets;
			for (int i = 0; i < segCount; i++)
			{
				LocationOffset rangeOffset;
				rangeOffset.bufferLocation = offset;
				rangeOffset.offset = ReadUint16Move(m, offset);
				idRangeOffsets.Add(rangeOffset);
			}
			
			for (int i = 0; i < segCount; i++)
			{
				uint16 endCode = endCodes[i];
				uint32 currentCode = (uint32)startCodes[i];

				while (currentCode <= endCode)
				{
					UnicodeToGlyphID UCodeToGlyphID{};
					// Range offset is 0, means we can calculate the glyphID from the existing data.
					if (idRangeOffsets[i].offset == 0)
					{
						UCodeToGlyphID.glyphID = (currentCode + idDeltas[i]) % 65536u;
					}
					else
					{
						uint16 rangeOffsetLocation = idRangeOffsets[i].bufferLocation + idRangeOffsets[i].offset;
						uint16 glyphIndexArrayLocation = 2 * (currentCode - startCodes[i]) + rangeOffsetLocation;

						uint16 glyphIndexOffset = ReadUint16(m, glyphIndexArrayLocation);

						if (glyphIndexOffset)
						{
							UCodeToGlyphID.glyphID = (glyphIndexOffset + idDeltas[i]) % 65536;
						}
					}
					UCodeToGlyphID.unicode = currentCode;

					maxUniCodeValueInSubFormat = Math::Max((int)currentCode, maxUniCodeValueInSubFormat);

					auto searchCondition = [&](const UnicodeToGlyphID& a) {
						if (a.unicode == UCodeToGlyphID.unicode)
							return true;
						return false;
					};

					if (outMappings.FindCustomCondition(searchCondition) == -1)
						outMappings.Add(UCodeToGlyphID);
					currentCode++;
				}
			}
		}
		return maxUniCodeValueInSubFormat;
	}

	int ReadCmap(char* mem, GrowingArray<UnicodeToGlyphID>& outMappings)
	{
		typedef struct {
			uint16 version;
			uint16 numberSubtables;
		} cmap;

		typedef struct {
			uint16 platformID;
			uint16 platformSpecificID;
			uint32 offset;
		} cmap_encoding_subtable;

		cmap c{};
		char* m = mem;

		uint32 offset = 0;

		c.version = ReadUint16(m, offset);
		offset += sizeof(uint16);
		c.numberSubtables = ReadUint16(m, offset);
		offset += sizeof(uint16);

		GrowingArray<cmap_encoding_subtable> subTables(c.numberSubtables);

		for (int i = 0; i < c.numberSubtables; ++i) 
		{
			cmap_encoding_subtable est{};
			est.platformID = ReadUint16(m, offset);
			offset += sizeof(uint16);
			est.platformSpecificID = ReadUint16(m, offset);
			offset += sizeof(uint16);
			est.offset = ReadUint32(m, offset);
			offset += sizeof(uint32);
			subTables.Add(est);
		}

		typedef struct {
			uint16 format;
			uint16 length;
			uint16 language;
		} subTableHeader;

		subTableHeader header;

		int maxUnicodeValue = -1;

		for (int i = 0; i < subTables.Size(); ++i) {
			printf("%d)\t%d\t%d\t%d\t", i + 1, subTables[i].platformID, subTables[i].platformSpecificID, subTables[i].offset);
			switch (subTables[i].platformID) {
			case 0: printf("Unicode"); break;
			case 1: printf("Mac"); break;
			case 2: printf("Not Supported"); break;
			case 3: printf("Microsoft"); break;
			}
			printf("\n");

			if (subTables[i].platformID == 0)
			{
				int subtableMaxUnicodeValue = ReadCmapSubFormat(mem + subTables[i].offset, outMappings);

				if (subtableMaxUnicodeValue > 0)
					maxUnicodeValue = Math::Max(maxUnicodeValue, subtableMaxUnicodeValue);
			}

			header.format = ReadUint16(m, subTables[i].offset);
			header.length = ReadUint16(m, subTables[i].offset + 2);
			header.language = ReadUint16(m, subTables[i].offset + 4);

		}
		return maxUnicodeValue;
	}

	TTF_FontStruct::GlyphData ParseGlyphs(char* pFontMemory, const GrowingArray<uint32>& glyphOffsets, Table glyfTable)
	{
		TTF_FontStruct::GlyphData glyphData;
		glyphData.m_maxNumbOfPrimitivesForGlyphs = 0;
		glyphData.m_maxNumbOfVerticesForGlyphs = 0;
		//uint32 currentGlyph = 0;
		for (uint32 iGlyph = 0; iGlyph < glyphOffsets.Size(); iGlyph++)
		{
			glyphData.m_glyphs.Add(ParseGlyph(pFontMemory + glyfTable.offset, glyphOffsets, glyphOffsets[iGlyph], glyphData.m_compoundGlyphs, glyphData.m_verts, glyphData.m_triangles, glyphData.m_maxNumbOfVerticesForGlyphs, glyphData.m_maxNumbOfPrimitivesForGlyphs, GlyphTransform()));
		}
		return glyphData;
	}

}

TTF_FontStruct Hail::TTF_ParseFontFile(const char* aFileToParse)
{
	InOutStream readStream;
	if (!readStream.OpenFile(FilePath(aFileToParse), FILE_OPEN_TYPE::READ, true))
	{
		H_ERROR(StringL::Format("Failed to open fontfile %s:", aFileToParse));
	}
	char* pFontMemory = new char[readStream.GetFileSize()];
	readStream.Read(pFontMemory, readStream.GetFileSize());
	readStream.CloseFile();

	FontDirectory directory;
	memcpy(&directory, pFontMemory, sizeof(FontDirectory));

	if (!IsSystemBigEndian())
	{
		SwapEndian(directory.scalerType);
		SwapEndian(directory.numTables);
		SwapEndian(directory.searchRange);
		SwapEndian(directory.entrySelector);
		SwapEndian(directory.rangeShift);
	}
	GrowingArray<Table> tables(directory.numTables);

	for (int i = 0; i < directory.numTables; i++)
	{
		Table tableToRead;
		memcpy(&tableToRead, pFontMemory + sizeof(FontDirectory) + sizeof(Table) * i, sizeof(Table));
		if (!IsSystemBigEndian())
		{
			SwapEndian(tableToRead.checkSum);
			SwapEndian(tableToRead.offset);
			SwapEndian(tableToRead.length);
		}
		tables.Add(tableToRead);

		uint8 tagString[5]{0};
		memcpy(tagString, tableToRead.tag.un.c, sizeof(uint32));
		H_DEBUGMESSAGE(StringL::Format("Table Entry %i: Tag: %s, offset: %u, length: %u", i, tagString, tableToRead.offset, tableToRead.length));
	}

	int16 glyphDataFormat = 0;
	uint16 numberOfGlyphs = 0;
	TTF_FontStruct font;

	for (size_t i = 0; i < tables.Size(); i++)
	{
		int8 tagString[5]{ 0 };
		memcpy(tagString, tables[i].tag.un.c, sizeof(uint32));
		if (StringCompare(tagString, "head"))
		{
			int16 xMin = ReadInt16(pFontMemory, tables[i].offset + 36);
			int16 yMin = ReadInt16(pFontMemory, tables[i].offset + 38);
			int16 xMax = ReadInt16(pFontMemory, tables[i].offset + 40);
			int16 yMax = ReadInt16(pFontMemory, tables[i].offset + 42);
			font.minGlyphBB = { xMin, yMin };
			font.maxGlyphBB = { xMax, yMax };
			glyphDataFormat = ReadInt16(pFontMemory, tables[i].offset + 50);
		}
		if (StringCompare(tagString, "maxp"))
		{
			numberOfGlyphs = ReadUint16(pFontMemory, tables[i].offset + sizeof(uint32));
		}
	}
	GrowingArray<uint32> glyphOffsets;
	glyphOffsets.PrepareAndFill(numberOfGlyphs);
	HHEATable hheaTable{};
	GrowingArray<UnicodeToGlyphID> uniCodeToGlyphMap;
	int maxUnicodeValue = -1;
	for (uint32 i = 0; i < tables.Size(); i++)
	{
		int8 tagString[5]{ 0 };
		memcpy(tagString, tables[i].tag.un.c, sizeof(uint32));
		if (StringCompare(tagString, "loca"))
		{
			for (uint32 iGlyphOffset = 0; iGlyphOffset < numberOfGlyphs; iGlyphOffset++)
			{
				if (glyphDataFormat == 0)
				{
					glyphOffsets[iGlyphOffset] = ReadUint16(pFontMemory, tables[i].offset + sizeof(uint16) * iGlyphOffset) * 2;
				}
				else
				{
					glyphOffsets[iGlyphOffset] = ReadUint32(pFontMemory, tables[i].offset + sizeof(uint32) * iGlyphOffset);
				}
			}
		}
		else if (StringCompare(tagString, "cmap"))
		{
			maxUnicodeValue = ReadCmap(pFontMemory + tables[i].offset, uniCodeToGlyphMap);
		}
		else if (StringCompare(tagString, "hhea"))
		{
			memcpy(&hheaTable, pFontMemory + tables[i].offset, sizeof(HHEATable));
		}
	}

	for (size_t i = 0; i < tables.Size(); i++)
	{
		int8 tagString[5]{ 0 };
		memcpy(tagString, tables[i].tag.un.c, sizeof(uint32));
		if (StringCompare(tagString, "glyf"))
		{
			font.m_glyphData = ParseGlyphs(pFontMemory, glyphOffsets, tables[i]);
		}

	}

	for (size_t i = 0; i < tables.Size(); i++)
	{
		int8 tagString[5]{ 0 };
		memcpy(tagString, tables[i].tag.un.c, sizeof(uint32));
		if (StringCompare(tagString, "hmtx"))
		{
			char* pHmtx = pFontMemory + tables[i].offset;
			uint32 oldOffset = 0;
			for (size_t iGlyph = 0; iGlyph < font.m_glyphData.m_glyphs.Size(); iGlyph++)
			{
				uint32 offset = tables[i].offset;
				uint16 advanceWidth = 0;
				int16 leftSideBearing = 0;
				if (iGlyph < hheaTable.numOfLongHorMetrics)
				{
					offset += iGlyph * 4;
					advanceWidth = ReadUint16(pFontMemory + offset, 0);
					leftSideBearing = ReadInt16(pFontMemory + offset, 2);
				}
				else 
				{
					// This code could be incredibly wrong
					// read the last entry of the hMetrics array
					H_DEBUGMESSAGE("Reading potentially incorrect code from font.");
					oldOffset = offset + (hheaTable.numOfLongHorMetrics - 1) * 4;
					advanceWidth = ReadUint16(pFontMemory + oldOffset, 0);
					offset = (offset + hheaTable.numOfLongHorMetrics * 4 + 2 * (iGlyph - hheaTable.numOfLongHorMetrics));
					leftSideBearing = ReadInt16(pFontMemory + offset, 0);
				}

				font.m_glyphData.m_glyphs[iGlyph].m_leftSideBearing = leftSideBearing;
				font.m_glyphData.m_glyphs[iGlyph].m_advanceWidth = advanceWidth;
			}
		}
	}



	// Remap Cmap coords to be more dirt cheap to check in runtime.
	if (maxUnicodeValue == -1)
	{
		//TODO: Add error handling, this will be zero if there are no Unicode to glyphID mappings in the file.
		//return false;
	}
	else
	{
		auto comparer = [&](const UnicodeToGlyphID& a, const UnicodeToGlyphID& b) { return a.unicode > b.unicode; };
		uniCodeToGlyphMap.Sort(comparer);
		font.m_uniCodeToGlyphID.PrepareAndFill(maxUnicodeValue);
		uint32 currentUCodeValFound = 0;

		for (size_t iCurrentUnicodeIndex = 0; iCurrentUnicodeIndex < maxUnicodeValue; iCurrentUnicodeIndex++)
		{
			font.m_uniCodeToGlyphID[iCurrentUnicodeIndex] = 0; // Setting the Unicode value to the invalid Glyph ID. 

			for (int i = currentUCodeValFound; i < uniCodeToGlyphMap.Size(); i++)
			{
				if (uniCodeToGlyphMap[i].unicode == iCurrentUnicodeIndex)
				{
					font.m_uniCodeToGlyphID[iCurrentUnicodeIndex] = uniCodeToGlyphMap[i].glyphID;
					currentUCodeValFound++;
					break;
				}
			}
		}
	}

	// Normalize all Glyph Coordinates:
	font.m_glyphExtents = font.maxGlyphBB - font.minGlyphBB;
	glm::ivec2 offsetToAddForNormalization = font.minGlyphBB * -1;
	glm::ivec2 normalizationDenominator = font.maxGlyphBB + offsetToAddForNormalization;
	for (size_t i = 0; i < font.m_glyphData.m_verts.Size(); i++)
	{
		glm::vec2& vert = font.m_glyphData.m_verts[i];
		vert += offsetToAddForNormalization;
		vert = glm::vec2(vert.x / normalizationDenominator.x, vert.y / normalizationDenominator.y);
	}
	
	for (size_t i = 0; i < font.m_glyphData.m_verts.Size(); i += 2)
	{
		glm::vec4 packedVert = glm::vec4(0.f);

		packedVert.x = font.m_glyphData.m_verts[i].x;
		packedVert.y = font.m_glyphData.m_verts[i].y;
		if (i + 1 < font.m_glyphData.m_verts.Size())
		{
			packedVert.z = font.m_glyphData.m_verts[i + 1].x;
			packedVert.w = font.m_glyphData.m_verts[i + 1].y;
		}
		font.m_renderVerts.Add(packedVert);
	}

	SAFEDELETE_ARRAY(pFontMemory);
	return font;
}
