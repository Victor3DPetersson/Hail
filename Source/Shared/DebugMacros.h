#pragma once

#include "Utilities.h"
#include "String.hpp"

inline void Debug_PrintConsoleConstChar(const char* string)
{
#ifdef DEBUG 
std::cout << string << std::endl;
#endif 
}


inline void Debug_PrintConsoleString256(String256 string) 
{
#ifdef DEBUG 
	std::cout << string.Data() << std::endl;
#endif 
}

inline void Debug_PrintConsoleString64(String64 string)
{
#ifdef DEBUG 
	std::cout << string.Data() << std::endl;
#endif 
}
