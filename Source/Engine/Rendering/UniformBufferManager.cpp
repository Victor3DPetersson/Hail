#include "Engine_PCH.h"
#include "UniformBufferManager.h"

namespace Hail
{

    const uint32_t GetUniformBufferIndex(const BUFFERS buffer)
    {
        switch (buffer)
        {
        case Hail::BUFFERS::TUTORIAL:
            return (uint32)BUFFERS::TUTORIAL;
            break;
        case Hail::BUFFERS::PER_FRAME_DATA:
            return (uint32)BUFFERS::PER_FRAME_DATA;
            break;
        case Hail::BUFFERS::SPRITE_INSTANCE_BUFFER:
            return (uint32)BUFFERS::SPRITE_INSTANCE_BUFFER;
            break;
        case Hail::BUFFERS::DEBUG_LINE_INSTANCE_BUFFER:
            return (uint32)BUFFERS::DEBUG_LINE_INSTANCE_BUFFER;
            break;
        case Hail::BUFFERS::PER_CAMERA_DATA:
            return (uint32)BUFFERS::PER_CAMERA_DATA;
            break;
        case Hail::BUFFERS::COUNT:
            return 0;
            break;
        default:
            return 0;
            break;
        }
    }
    constexpr uint32 BUFFER_TUTORIAL_SIZE = sizeof(TutorialUniformBufferObject);
    constexpr uint32 BUFFER_SPRITE_SIZE = sizeof(SpriteInstanceData) * MAX_NUMBER_OF_SPRITES;
    constexpr uint32 BUFFER_PER_FRAME_SIZE = sizeof(PerFrameUniformBuffer);
    constexpr uint32 BUFFER_PER_CAMERA_SIZE = sizeof(PerCameraUniformBuffer);
    constexpr uint32 BUFFER_DEBUG_LINE_SIZE = sizeof(DebugLineData) * MAX_NUMBER_OF_DEBUG_LINES;
    const uint32_t GetUniformBufferSize(const BUFFERS buffer)
    {
        //assert if buffer is count;
        switch (buffer)
        {
        case BUFFERS::TUTORIAL:
        {
            return BUFFER_TUTORIAL_SIZE;
        }
        break;
        case BUFFERS::PER_FRAME_DATA: 
        {
            return BUFFER_PER_FRAME_SIZE;
        }
        break;
        case BUFFERS::PER_CAMERA_DATA:
        {
            return BUFFER_PER_CAMERA_SIZE;
        }
        break;
        case BUFFERS::SPRITE_INSTANCE_BUFFER:
        {
            return BUFFER_SPRITE_SIZE;
        }
        break;

        case BUFFERS::DEBUG_LINE_INSTANCE_BUFFER:
        {
            return BUFFER_DEBUG_LINE_SIZE;
        }
        break;

        case BUFFERS::COUNT:
            return 0;
            break;
        default:
            return 0;
            break;
        }
    }


}