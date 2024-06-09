#include "Engine_PCH.h"
#include "BufferResource.h"

using namespace Hail;

const uint32 Hail::BufferObject::GetBufferSize() const
{
    return m_properties.elementByteSize * m_properties.numberOfElements;
}
