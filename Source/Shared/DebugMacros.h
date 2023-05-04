#pragma once

#include "Utilities.h"
#include "String.hpp"
#ifdef DEBUG
#include <iostream>
#endif

inline void Debug_PrintConsoleConstChar([[maybe_unused]]const char* string)
{
#ifdef DEBUG 
std::cout << string << std::endl;
#endif 
}


inline void Debug_PrintConsoleString256([[maybe_unused]] String256 string)
{
#ifdef DEBUG 
	std::cout << string.Data() << std::endl;
#endif 
}

inline void Debug_PrintConsoleString64([[maybe_unused]] String64 string)
{
#ifdef DEBUG 
	std::cout << string.Data() << std::endl;
#endif 
}
