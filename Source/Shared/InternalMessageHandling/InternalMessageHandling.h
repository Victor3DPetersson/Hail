#pragma once
#include "String.hpp"

#ifdef DEBUG
#include <cassert>
#include <iostream>
#endif

namespace Hail
{
	enum class eMessageType : uint8
	{
		LogMessage, // Messages that will be written to the output and the log window
		Warning, // Warnings are for when something is not going as expecting but it might not be dangerous for the applicaton.
		Error, // Errors are for content or code going wrong, can be things like that a resource could not be found or is missing.
		Fatal, // Asserts and or game breaking errors.
		Count
	};

	void CreateMessage(const char* message, const char* fileName, int line, eMessageType type);
	

#ifdef DEBUG
	void AssertMessage(const char* expr_str, bool expression, const char* file, int line, const char* message);

// from Stack-overflow on how to get a default parameter in a macro.
// The multiple macros that you would need anyway [as per: Crazy Eddie]
#define H_ASSERT_1(Condition) Hail::AssertMessage(#Condition, Condition, __FILE__, __LINE__, "" );
#define H_ASSERT_2(Condition,Message) Hail::AssertMessage(#Condition, Condition, __FILE__, __LINE__, Message );

// The interim macro that simply strips the excess and ends up with the required macro
#define H_ASSERT_X(x,Condition,Message,FUNC, ...)  FUNC  

// The macro that the programmer uses 
#define H_ASSERT(...)                    H_ASSERT_X(,##__VA_ARGS__,\
                                          H_ASSERT_2(__VA_ARGS__),\
                                          H_ASSERT_1(__VA_ARGS__)\
                                         ) 


//#define H_ASSERT(Condition,Message) Hail::AssertMessage(#Condition, Condition, __FILE__, __LINE__, Message );
//#define H_ASSERT(Condition) Hail::AssertMessage(#Condition, Condition, __FILE__, __LINE__, "" );

#define H_ASSERT_LOGMESSAGE(Condition,Message) \
	H_ASSERT(Condition,Message)\
	if (!Condition)	\
	{ \
	Hail::CreateMessage(Message, __FILE__, __LINE__, eMessageType::Fatal); \
} \

#define H_ERROR(Message) { Hail::CreateMessage(Message, __FILE__, __LINE__, eMessageType::Error); }
#define H_WARNING(Message) { Hail::CreateMessage(Message, __FILE__, __LINE__, eMessageType::Warning); }
#define H_DEBUGMESSAGE(Message) { Hail::CreateMessage(Message, __FILE__, __LINE__, eMessageType::LogMessage); }


#else
#define H_ASSERT(Condition, Message) {}
#define H_ASSERT_LOGMESSAGE(Condition, Message) {}
#define H_ERROR(Message) {}
#define H_WARNING(Message) {}
#define H_DEBUGMESSAGE(Message) {}
#endif



}