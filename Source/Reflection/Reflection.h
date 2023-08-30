#pragma once
#include <iostream>
#include "String.hpp"
#include "ReflectionDefines.h"
namespace Hail
{
	namespace Reflection
	{
		constexpr unsigned int MAX_NUMBER_OF_FIELDS = 64u;
		struct Type {
			String64 name;
			size_t size = 0;
		};

		struct Field {
			const Type* type;
			String64 name;
			size_t offset = 0;
		};

		struct Class {
			StaticArray<Field, MAX_NUMBER_OF_FIELDS> fields;
			uint16_t numberOfFields = 0;
		};

		template<typename T>
		Type* GetType();

#define DEFINE_TYPE(TYPE) \
	template<> \
	Type* GetType<TYPE>() { \
	static Type type; \
	type.name = #TYPE; \
	type.size = sizeof(TYPE); \
	return &type; \
	}\

		template<typename T>
		const Class* GetClass();

#define BEGIN_ATTRIBUTES_FOR(CLASS)  \
	template<> \
	const Class* GetClass<CLASS>() { \
		using ClassType = CLASS; \
		static Class localClass; \
		enum { BASE = __COUNTER__ }; \

#define DEFINE_MEMBER(NAME, TYPE)  \
		enum { NAME##Index = __COUNTER__ - BASE - 1}; \
		localClass.fields[NAME##Index].type = GetType<TYPE>();\
		localClass.fields[NAME##Index].name = { #NAME };  \
		localClass.fields[NAME##Index].offset = offsetof(ClassType, NAME);\
		localClass.numberOfFields = NAME##Index; \

#define END_ATTRIBUTES \
		localClass.numberOfFields++; \
	return &localClass; \
	}\

	}

}

//Example of how to use the reflection macros
//REFLECTABLE_CLASS()
//struct AnotherTestStruct {
//	REFLECTABLE_MEMBER()
//		int32 field1 = 0;
//	REFLECTABLE_MEMBER()
//		int16 field2 = 0;
//	REFLECTABLE_MEMBER()
//		int8 field3 = 0;
//	REFLECTABLE_MEMBER()
//		uint32 field4 = 0;
//	REFLECTABLE_MEMBER()
//		uint16 field5 = 0;
//	REFLECTABLE_MEMBER()
//		bool field6 = false;
//};


