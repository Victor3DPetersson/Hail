#include "Engine_PCH.h"
#include "UniformBufferManager.h"

namespace Hail
{

    const uint32_t GetUniformBufferIndex(const BUFFERS buffer)
    {
        switch (buffer)
        {
        case Hail::BUFFERS::TUTORIAL:
            return static_cast<uint32_t>(BUFFERS::TUTORIAL);
            break;
        case Hail::BUFFERS::PER_FRAME_DATA:
            return static_cast<uint32_t>(BUFFERS::PER_FRAME_DATA);
            break;
        case Hail::BUFFERS::SPRITE_INSTANCE_BUFFER:
            return static_cast<uint32_t>(BUFFERS::SPRITE_INSTANCE_BUFFER);
            break;
        case Hail::BUFFERS::COUNT:
            return 0;
            break;
        default:
            return 0;
            break;
        }
    }
    const uint32_t GetUniformBufferSize(const BUFFERS buffer)
    {
        //assert if buffer is count;
        switch (buffer)
        {
        case Hail::BUFFERS::TUTORIAL:
            return sizeof(TutorialUniformBufferObject);
            break;
        case Hail::BUFFERS::PER_FRAME_DATA:
            return sizeof(PerFrameUniformBuffer);
            break;
        case Hail::BUFFERS::SPRITE_INSTANCE_BUFFER:
            return sizeof(SpriteInstanceDataBuffer);
            break;
        case Hail::BUFFERS::COUNT:
            return 0;
            break;
        default:
            return 0;
            break;
        }
    }


}