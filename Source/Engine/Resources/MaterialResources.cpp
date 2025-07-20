#include "Engine_PCH.h"
#include "MaterialResources.h"

using namespace Hail;

Hail::uint64 Hail::GetMaterialSortValue(eMaterialType type, eBlendMode blend, uint64 shaderValues)
{
	const uint32 combinedTypeAndBlend = ((uint8)type << 0xf) | (uint8)blend; // Combine the two values to 2 bits as there are not many combinations
	return shaderValues << 8 | combinedTypeAndBlend;
}

Hail::MaterialTypeObject::BindingInformation::BindingInformation()
{
	ClearBindings();
}

void Hail::MaterialTypeObject::BindingInformation::ClearBindings()
{
	for (uint32 iFrame = 0; iFrame < MAX_FRAMESINFLIGHT; iFrame++)
	{
		for (uint32 iDecoration = 0; iDecoration < (uint32)eDecorationType::Count; iDecoration++)
		{
			m_boundResourceLists[iFrame][iDecoration].Fill(MAX_UINT);
		}
	}
}

StaticArray<uint32, MaxShaderBindingCount>& Hail::MaterialTypeObject::BindingInformation::GetDecorationBoundList(uint32 frameInFlight, eDecorationType decoration)
{
	return m_boundResourceLists[frameInFlight][(uint32)decoration];
}
