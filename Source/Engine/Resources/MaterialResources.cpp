#include "Engine_PCH.h"
#include "MaterialResources.h"

Hail::uint64 Hail::GetMaterialSortValue(eMaterialType type, eBlendMode blend, uint64 shaderValues)
{
	const uint32 combinedTypeAndBlend = ((uint8)type << 0xf) | (uint8)blend; // Combine the two values to 2 bits as there are not many combinations
	return shaderValues << 8 | combinedTypeAndBlend;
}

