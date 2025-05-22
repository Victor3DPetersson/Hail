#pragma once
#include "glm\vec2.hpp"
#include "glm\vec3.hpp"
#include "glm\vec4.hpp"

namespace Hail
{
    using uint8 = unsigned char;
    using uint16 = unsigned short;
    using uint32 = unsigned int;
    using uint64 = unsigned long long;

    using int8 = char;
    using int16 = short;
    using int32 = int;
    using int64 = long long;

    using float32 = float;
    using float64 = double;

    constexpr uint16 MAX_UINT16 = 0xFFFF;
    constexpr uint32 MAX_UINT = 0xFFFFFFFF;
    constexpr uint64 MAX_UINT64 = 0xFFFFFFFFFFFFFFFF;

    constexpr float MAX_FLOAT = std::numeric_limits<float>::max();
    constexpr double MAX_DOUBLE = std::numeric_limits<double>::max();

    struct GUID {
        bool operator ==(const GUID& other) const
        {
            return m_data1 == other.m_data1 && m_data2 == other.m_data2 && m_data2 == other.m_data2 && m_data3 == other.m_data3
                && m_data4[0] == other.m_data4[0] && m_data4[1] == other.m_data4[1] && m_data4[2] == other.m_data4[2] && m_data4[3] == other.m_data4[3]
                && m_data4[4] == other.m_data4[4] && m_data4[5] == other.m_data4[5] && m_data4[6] == other.m_data4[6] && m_data4[7] == other.m_data4[7];
        }
        bool operator !=(const GUID& other) const
        {
            return !((*this) == other);
        }
        unsigned long  m_data1{};
        unsigned short m_data2{};
        unsigned short m_data3{};
        unsigned char  m_data4[8]{};
    };

    static GUID GuidZero = GUID();

    static glm::vec2 Vec2Zero = glm::vec2(0.0, 0.0);
    static glm::vec2 Vec2One = glm::vec2(1.0, 1.0);

    static glm::vec3 Vec3Zero = glm::vec3(0.0, 0.0, 0.0);
    static glm::vec3 Vec3One = glm::vec3(1.0, 1.0, 1.0);

    static glm::vec4 Vec4Zero = glm::vec4(0.0, 0.0, 0.0, 0.0);
    static glm::vec4 Vec4One = glm::vec4(1.0, 1.0, 1.0, 1.0);

    constexpr uint32_t INVALID_UINT = 0xffffffff;

    enum class eResolutions : uint8
    {
        res2160,
        res1440,
        res1080,
        res720,
        res480,
        res360,
        Count
    };

    enum class eResourceState
    {
        Loaded,
        Unloaded, // Pending load
        Invalid // Failed to load
    };
}
