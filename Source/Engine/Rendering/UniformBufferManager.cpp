#include "Engine_PCH.h"
#include "UniformBufferManager.h"

namespace Hail
{

    const uint32_t GetUniformBufferIndex(const UNIFORM_BUFFERS buffer)
    {
        //assert if buffer is count;
        return static_cast<uint32_t>(buffer);
    }
    const uint32_t GetUniformBufferSize(const UNIFORM_BUFFERS buffer)
    {
        //assert if buffer is count;
        switch (buffer)
        {
        case Hail::UNIFORM_BUFFERS::TUTORIAL:
            return sizeof(TutorialUniformBufferObject);
            break;
        case Hail::UNIFORM_BUFFERS::PER_FRAME_DATA:
            return sizeof(PerFrameUniformBuffer);
            break;
        case Hail::UNIFORM_BUFFERS::COUNT:
            return 0;
            break;
        default:
            return 0;
            break;
        }
    }


}