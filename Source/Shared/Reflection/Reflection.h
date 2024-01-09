#pragma once
#include <iostream>
#include "String.hpp"
#include "ReflectionDefines.h"
#include "Containers\StaticArray\StaticArray.h"
#include "Types.h"
#include "SerializationOverride.h"
#include <type_traits>

namespace Hail
{
	namespace Reflection
	{
		//Base types predefined
		// bool
		// uint8
		// uint16
		// uint32
		// uint64
		// int8
		// int16
		// int32
		// int64
		// float32
		// float64

		constexpr unsigned int MAX_NUMBER_OF_FIELDS = 64u;
		struct Type {
			String64 name;
			size_t size = 0;
			bool usesCustomSerializer = false;
		};

		struct Field {
			const Type* type;
			String64 name;
			size_t offset = 0;
		};

		struct Class {
			StaticArray<Field, MAX_NUMBER_OF_FIELDS> fields;
			uint16_t numberOfFields = 0;
			bool usesCustomSerialization = false;
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

#define DEFINE_TYPE_CUSTOM_SERIALIZER(TYPE) \
		template<> \
		Type* GetType<TYPE>() { \
		TYPE typePureObject; \
		static Type type; \
		static_assert(std::is_polymorphic<TYPE>::value, "Object T must inherit from the SerializeableObjectCustom"); \
		type.name = #TYPE; \
		type.size = sizeof(TYPE); \
		type.usesCustomSerializer = dynamic_cast<SerializeableObjectCustom*>(&typePureObject); \
		return &type; \
		}\


		template<typename T>
		const Class* GetClass();

#define BEGIN_ATTRIBUTES_FOR(CLASS)  \
		template<typename T> \
		const Class* GetClass<CLASS>() { \
		using ClassType = CLASS; \
		static Class localClass; \
		localClass.usesCustomSerialization = false; \
		enum { BASE = __COUNTER__ }; \

#define DEFINE_MEMBER(NAME, TYPE)  \
		enum { NAME##Index = __COUNTER__ - BASE - 1}; \
		localClass.fields[NAME##Index].type = GetType<TYPE>();\
		localClass.fields[NAME##Index].name = { #NAME };  \
		localClass.fields[NAME##Index].offset = offsetof(ClassType, NAME);\
		localClass.numberOfFields = NAME##Index; \

#define END_ATTRIBUTES \
		localClass.numberOfFields++; \
		static_assert(std::is_polymorphic<T>::value, "Object T must inherit from the SerializeableObjectCustom"); \
		T classInstance{}; \
		if (dynamic_cast<SerializeableObjectCustom*>(&classInstance)) { localClass.usesCustomSerialization = true; } \
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


