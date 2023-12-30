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

    constexpr uint32 MAX_UINT = 0xFFFFFFFF;
    constexpr uint16 MAX_UINT16 = 0xFFFF;

    struct GUID {
        unsigned long  Data1{};
        unsigned short Data2{};
        unsigned short Data3{};
        unsigned char  Data4[8]{};
    };

    static glm::vec2 Vec2Zero = glm::vec2(0.0, 0.0);
    static glm::vec2 Vec2One = glm::vec2(1.0, 1.0);

    static glm::vec3 Vec3Zero = glm::vec3(0.0, 0.0, 0.0);
    static glm::vec3 Vec3One = glm::vec3(1.0, 1.0, 1.0);

    static glm::vec4 Vec4Zero = glm::vec4(0.0, 0.0, 0.0, 0.0);
    static glm::vec4 Vec4One = glm::vec4(1.0, 1.0, 1.0, 1.0);
}
