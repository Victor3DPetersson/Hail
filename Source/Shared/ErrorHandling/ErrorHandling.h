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

	void CreateMessage(String256 message, String256 fileName, int line, eMessageType type);
	

#ifdef DEBUG

	void AssertMessage(const char* expr_str, bool expression, const char* file, int line, const char* message);
#define H_ASSERT(Condition,Message) AssertMessage(#Condition, Condition, __FILE__, __LINE__, Message );

#define H_ASSERT_LOGMESSAGE(Condition,Message) \
	H_ASSERT(Condition,Message)\
	if (!Condition)	\
	{ \
	CreateMessage(Message, __FILE__, __LINE__, eMessageType::Fatal); \
} \

#define H_ERROR(Message) CreateMessage(Message, __FILE__, __LINE__, eMessageType::Error);
#define H_WARNING(Message) CreateMessage(Message, __FILE__, __LINE__, eMessageType::Warning);
#define H_DEBUGMESSAGE(Message) CreateMessage(Message, __FILE__, __LINE__, eMessageType::LogMessage);


#else
#define H_ASSERT(Condition, Message)
#define H_ASSERT_LOGMESSAGE(Condition, Message)
#define H_ERROR(Message)
#define H_WARNING(Message)
#define H_DEBUGMESSAGE(Message)
#endif



}