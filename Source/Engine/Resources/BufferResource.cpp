#include "Engine_PCH.h"
#include "BufferResource.h"

using namespace Hail;

uint32 BufferObject::g_idCounter = 0;

bool Hail::BufferObject::Init(RenderingDevice* pDevice, BufferProperties properties)
{
	m_properties = properties;

	bool bIsAValidBuffer = false;

	if (properties.usage != eShaderBufferUsage::Undefined && properties.domain != eShaderBufferDomain::Undefined)
	{
		bIsAValidBuffer = true;
		if (properties.domain != eShaderBufferDomain::GpuOnly && properties.updateFrequency == eShaderBufferUpdateFrequency::Undefined)
			bIsAValidBuffer = false;

		if (properties.elementByteSize == 0 || properties.numberOfElements == 0)
			bIsAValidBuffer = false;
	}
	H_ASSERT(bIsAValidBuffer, "Invalid buffer");
	if (!bIsAValidBuffer)
		return false;


	if (m_properties.updateFrequency == eShaderBufferUpdateFrequency::Once)
		m_bUsesFramesInFlight = false;
	else
		m_bUsesFramesInFlight = true;

	const bool result = InternalInit(pDevice);

	if (result)
		m_id = g_idCounter++;

	return result;
}

const uint32 Hail::BufferObject::GetBufferSize() const
{
    return m_properties.elementByteSize * m_properties.numberOfElements;
}
