#pragma once
#include "Types.h"
#include "glm\vec3.hpp"
#include "glm\gtx\color_encoding.hpp"

namespace Hail
{
#ifdef PLATFORM_WINDOWS
    const uint32 MAX_FRAMESINFLIGHT = 2;
#endif

    enum class TEXTURE_FORMAT : uint32
    {
        UNDEFINED = 0,
        R4G4_UNORM_PACK8 = 1,
        R4G4B4A4_UNORM_PACK16 = 2,
        B4G4R4A4_UNORM_PACK16 = 3,
        R5G6B5_UNORM_PACK16 = 4,
        B5G6R5_UNORM_PACK16 = 5,
        R5G5B5A1_UNORM_PACK16 = 6,
        B5G5R5A1_UNORM_PACK16 = 7,
        A1R5G5B5_UNORM_PACK16 = 8,
        R8_UNORM = 9,
        R8_SNORM = 10,
        R8_USCALED = 11,
        R8_SSCALED = 12,
        R8_UINT = 13,
        R8_SINT = 14,
        R8_SRGB = 15,
        R8G8_UNORM = 16,
        R8G8_SNORM = 17,
        R8G8_USCALED = 18,
        R8G8_SSCALED = 19,
        R8G8_UINT = 20,
        R8G8_SINT = 21,
        R8G8_SRGB = 22,
        R8G8B8_UNORM = 23,
        R8G8B8_SNORM = 24,
        R8G8B8_USCALED = 25,
        R8G8B8_SSCALED = 26,
        R8G8B8_UINT = 27,
        R8G8B8_SINT = 28,
        R8G8B8_SRGB = 29,
        B8G8R8_UNORM = 30,
        B8G8R8_SNORM = 31,
        B8G8R8_USCALED = 32,
        B8G8R8_SSCALED = 33,
        B8G8R8_UINT = 34,
        B8G8R8_SINT = 35,
        B8G8R8_SRGB = 36,
        R8G8B8A8_UNORM = 37,
        R8G8B8A8_SNORM = 38,
        R8G8B8A8_USCALED = 39,
        R8G8B8A8_SSCALED = 40,
        R8G8B8A8_UINT = 41,
        R8G8B8A8_SINT = 42,
        R8G8B8A8_SRGB = 43,
        B8G8R8A8_UNORM = 44,
        B8G8R8A8_SNORM = 45,
        B8G8R8A8_USCALED = 46,
        B8G8R8A8_SSCALED = 47,
        B8G8R8A8_UINT = 48,
        B8G8R8A8_SINT = 49,
        B8G8R8A8_SRGB = 50,
        A8B8G8R8_UNORM_PACK32 = 51,
        A8B8G8R8_SNORM_PACK32 = 52,
        A8B8G8R8_USCALED_PACK32 = 53,
        A8B8G8R8_SSCALED_PACK32 = 54,
        A8B8G8R8_UINT_PACK32 = 55,
        A8B8G8R8_SINT_PACK32 = 56,
        A8B8G8R8_SRGB_PACK32 = 57,
        A2R10G10B10_UNORM_PACK32 = 58,
        A2R10G10B10_SNORM_PACK32 = 59,
        A2R10G10B10_USCALED_PACK32 = 60,
        A2R10G10B10_SSCALED_PACK32 = 61,
        A2R10G10B10_UINT_PACK32 = 62,
        A2R10G10B10_SINT_PACK32 = 63,
        A2B10G10R10_UNORM_PACK32 = 64,
        A2B10G10R10_SNORM_PACK32 = 65,
        A2B10G10R10_USCALED_PACK32 = 66,
        A2B10G10R10_SSCALED_PACK32 = 67,
        A2B10G10R10_UINT_PACK32 = 68,
        A2B10G10R10_SINT_PACK32 = 69,
        R16_UNORM = 70,
        R16_SNORM = 71,
        R16_USCALED = 72,
        R16_SSCALED = 73,
        R16_UINT = 74,
        R16_SINT = 75,
        R16_SFLOAT = 76,
        R16G16_UNORM = 77,
        R16G16_SNORM = 78,
        R16G16_USCALED = 79,
        R16G16_SSCALED = 80,
        R16G16_UINT = 81,
        R16G16_SINT = 82,
        R16G16_SFLOAT = 83,
        R16G16B16_UNORM = 84,
        R16G16B16_SNORM = 85,
        R16G16B16_USCALED = 86,
        R16G16B16_SSCALED = 87,
        R16G16B16_UINT = 88,
        R16G16B16_SINT = 89,
        R16G16B16_SFLOAT = 90,
        R16G16B16A16_UNORM = 91,
        R16G16B16A16_SNORM = 92,
        R16G16B16A16_USCALED = 93,
        R16G16B16A16_SSCALED = 94,
        R16G16B16A16_UINT = 95,
        R16G16B16A16_SINT = 96,
        R16G16B16A16_SFLOAT = 97,
        R32_UINT = 98,
        R32_SINT = 99,
        R32_SFLOAT = 100,
        R32G32_UINT = 101,
        R32G32_SINT = 102,
        R32G32_SFLOAT = 103,
        R32G32B32_UINT = 104,
        R32G32B32_SINT = 105,
        R32G32B32_SFLOAT = 106,
        R32G32B32A32_UINT = 107,
        R32G32B32A32_SINT = 108,
        R32G32B32A32_SFLOAT = 109,
        R64_UINT = 110,
        R64_SINT = 111,
        R64_SFLOAT = 112,
        R64G64_UINT = 113,
        R64G64_SINT = 114,
        R64G64_SFLOAT = 115,
        R64G64B64_UINT = 116,
        R64G64B64_SINT = 117,
        R64G64B64_SFLOAT = 118,
        R64G64B64A64_UINT = 119,
        R64G64B64A64_SINT = 120,
        R64G64B64A64_SFLOAT = 121,
        B10G11R11_UFLOAT_PACK32 = 122,
        E5B9G9R9_UFLOAT_PACK32 = 123

    };
    enum class TEXTURE_DEPTH_FORMAT : uint32_t
    {
        UNDEFINED = 0,
        D16_UNORM = 1,
        X8_D24_UNORM_PACK32 = 2,
        D32_SFLOAT = 3,
        S8_UINT = 4,
        D16_UNORM_S8_UINT = 5,
        D24_UNORM_S8_UINT = 6,
        D32_SFLOAT_S8_UINT = 7
    };


	enum class TEXTURE_WRAP_MODE : uint32_t
	{
        REPEAT = 0,
        MIRRORED_REPEAT = 1,
        CLAMP_TO_EDGE = 2,
        CLAMP_TO_BORDER = 3,
        MIRROR_CLAMP_TO_EDGE = 4
	};

    enum class TEXTURE_FILTER_MODE : uint32_t
    {
        NEAREST = 0,
        LINEAR = 1,
        CUBIC_EXT = 2
    };

    enum class TEXTURE_SAMPLER_FILTER_MODE : uint32_t
    {
        POINT = 0,
        LINEAR = 1
    };

    enum class COMPARE_MODE : uint32_t
    {
       NEVER = 0,
       LESS = 1,
       EQUAL = 2,
       LESS_OR_EQUAL = 3,
       GREATER = 4,
       NOT_EQUAL = 5,
       GREATER_OR_EQUAL = 6,
       ALWAYS = 7
    };

    constexpr glm::vec3 Color_RED = { 1.0, 0.0, 0.0 };
    constexpr glm::vec3 Color_BLUE = { 0.0, 0.0, 1.0 };
    constexpr glm::vec3 Color_GREEN = { 0.0, 1.0, 0.0 };
    constexpr glm::vec3 Color_BLACK = { 0.0, 0.0, 0.0 };
    constexpr glm::vec3 Color_WHITE = { 1.0, 1.0, 1.0 };
    constexpr glm::vec3 Color_MAGENTA = { 1.0, 0.0, 1.0 };
    constexpr glm::vec3 Color_CYAN = { 0.0, 1.0, 1.0 };
    constexpr glm::vec3 Color_YELLOW = { 1.0, 1.0, 0.0 };

    struct TextureSamplerData
    {
        TEXTURE_WRAP_MODE wrapMode_u = TEXTURE_WRAP_MODE::REPEAT;
        TEXTURE_WRAP_MODE wrapMode_v = TEXTURE_WRAP_MODE::REPEAT;
        TEXTURE_WRAP_MODE wrapMode_w = TEXTURE_WRAP_MODE::REPEAT;
        TEXTURE_FILTER_MODE filter_mag = TEXTURE_FILTER_MODE::NEAREST;
        TEXTURE_FILTER_MODE filter_min = TEXTURE_FILTER_MODE::NEAREST;
        TEXTURE_SAMPLER_FILTER_MODE sampler_mode = TEXTURE_SAMPLER_FILTER_MODE::LINEAR;
        COMPARE_MODE compareOp = COMPARE_MODE::ALWAYS;
        glm::vec3 borderColor = Color_BLACK;
        bool anisotropy = true;
    };

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
        bool m_resourceIsDirty = false;
        uint32 m_frameInFlightThatSetDirty = 0;
    };
}