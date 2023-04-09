#pragma once
#include <stdint.h>


namespace Hail
{
	constexpr uint32_t MAX_NUMBER_OF_SPRITES = 1024;

    struct GUID {
        unsigned long  Data1{};
        unsigned short Data2{};
        unsigned short Data3{};
        unsigned char  Data4[8]{};
    };
}

