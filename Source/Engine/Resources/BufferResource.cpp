#include "Engine_PCH.h"
#include "BufferResource.h"

using namespace Hail;

uint32 BufferObject::g_idCounter = 0;

const uint32 Hail::BufferObject::GetBufferSize() const
{
    return m_properties.elementByteSize * m_properties.numberOfElements;
}
