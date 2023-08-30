#include "Reflection_PCH.h"
#include "Reflection.h"

namespace Hail
{
	namespace Reflection
	{
		//Defining base types
		DEFINE_TYPE(bool)
		DEFINE_TYPE(int8)
		DEFINE_TYPE(int16)
		DEFINE_TYPE(int32)
		DEFINE_TYPE(int64)
		DEFINE_TYPE(uint8)
		DEFINE_TYPE(uint16)
		DEFINE_TYPE(uint32)
		DEFINE_TYPE(uint64)
		DEFINE_TYPE(float32)
		DEFINE_TYPE(float64)
		DEFINE_TYPE(String64)
		DEFINE_TYPE(String256)
		//If adding more types, add them too the CppParser.cpp file as well in the BASIC_TYPES
	}
}

