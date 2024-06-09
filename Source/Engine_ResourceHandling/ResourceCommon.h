#pragma once
#include "Types.h"
#include "glm\vec3.hpp"
#include "glm\gtx\color_encoding.hpp"

namespace Hail
{
#ifdef PLATFORM_WINDOWS
    const uint32 MAX_FRAMESINFLIGHT = 2;
#endif

    // A class that keeps track if a rendering resource is dirty or not, 
    // works with the FramesInFlight to make sure that the frame in flights can keep track of what is dirty or not.
    // On creation nothing is dirty, no need to set anything up with this class.
    class ResourceValidator
    {
    public:
        ResourceValidator();
        void MarkResourceAsDirty(uint32 frameInFlight);
        bool IsAllFrameResourcesDirty() const;
        uint32 GetFrameThatMarkedFrameDirty() const { return m_frameInFlightThatSetDirty; }
        
        bool GetIsFrameDataDirty(uint32 frameInFlight) const { return m_dirtyFrameData[frameInFlight]; }
        bool GetIsResourceDirty() const { return m_resourceIsDirty; }
        void ClearFrameData(uint32 frameInFlight);
    private:

        bool m_dirtyFrameData[MAX_FRAMESINFLIGHT];
        //Variable to mark that we have started a reload of the resource
        bool m_resourceIsDirty;
        uint32 m_frameInFlightThatSetDirty;
    };
}