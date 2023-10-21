#pragma once

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

    struct GUID {
        unsigned long  Data1{};
        unsigned short Data2{};
        unsigned short Data3{};
        unsigned char  Data4[8]{};
    };
}
