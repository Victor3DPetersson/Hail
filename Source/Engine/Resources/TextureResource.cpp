#include "Engine_PCH.h"
#include "TextureResource.h"

using namespace Hail;

uint32 TextureResource::g_idCounter = 0;

bool Hail::TextureResource::Init(RenderingDevice* pDevice)
{
    if (m_properties.width == 0 || m_properties.height == 0)
    {
        H_ERROR("Invalid texture dimensions");
        return false;
    }

    if (m_index != MAX_UINT)
        return InternalInit(pDevice);

    return false;
}
