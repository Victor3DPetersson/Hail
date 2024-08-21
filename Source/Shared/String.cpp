#pragma once
#include "Shared_PCH.h"
#include <string.h>
#include <string>
#include <assert.h>
#include <stdarg.h>

#include "Utility/StringUtility.h"
#include "String.hpp"
#include "Types.h"
#include "InternalMessageHandling\InternalMessageHandling.h"
#include "StringMemoryAllocator.h"

using namespace Hail;

constexpr uint32 l = sizeof(char*);
Hail::StringL::StringL()
	: m_length(0)
{
	m_memory.m_p = (nullptr);
}

Hail::StringL::StringL(const char* const string)
{
	m_length = StringLength(string);
	if (m_length > 15)
	{
		StringMemoryAllocator::GetInstance().AllocateString(string, m_length, &m_memory.m_p);
	}
	else
	{
		strcpy_s(m_memory.m_shortString, string);
	}
}

Hail::StringL::StringL(const StringL& anotherString)
{
	m_memory.m_p = nullptr;
	if (anotherString.Length() > 15)
	{
		StringMemoryAllocator::GetInstance().AllocateString(anotherString.m_memory.m_p, anotherString.m_length, &m_memory.m_p);
	}
	else
	{
		strcpy_s(m_memory.m_shortString, anotherString.m_memory.m_shortString);
	}
	m_length = anotherString.Length();
}

Hail::StringL::StringL(const String64& string64)
{
	m_length = string64.Length();
	if (m_length > 15)
	{
		StringMemoryAllocator::GetInstance().AllocateString(string64.Data(), m_length, &m_memory.m_p);
	}
	else
	{
		strcpy_s(m_memory.m_shortString, string64.Data());
	}
}

Hail::StringL::StringL(StringL&& moveableString)
{
	m_length = moveableString.m_length;
	if (moveableString.Length() > 15)
	{
		StringMemoryAllocator::GetInstance().MoveStringAllocator(&moveableString.m_memory.m_p, &m_memory.m_p);
		moveableString.m_length = 0;
	}
	else
	{
		strcpy_s(m_memory.m_shortString, moveableString.m_memory.m_shortString);
		//moveableString.m_memory.m_shortString[0] = 0;
	}
}

Hail::StringL::StringL(const std::string& stlString)
{
	H_ERROR("Use of stl string.");
	m_length = stlString.size();
	if (stlString.size() > 15)
	{
		StringMemoryAllocator::GetInstance().AllocateString(stlString.c_str(), m_length, &m_memory.m_p);
	}
	else
	{
		strcpy_s(m_memory.m_shortString, stlString.c_str());
	}
}

Hail::StringL::~StringL()
{
	if (m_length > 15)
		StringMemoryAllocator::GetInstance().DeallocateString(&m_memory.m_p);

	m_memory.m_p = nullptr;
	m_length = 0;
}

StringL Hail::StringL::Format(const char* const format, ...)
{
	uint32 length = StringLength(format);
	size_t len = 0;
	va_list argp;
	char* p;

	if (format == NULL)
		return StringL();

	int intArgument;
	float floatArgument;
	uint32 uintArgument;
	char* stringArgument;
	char numberCharString[64];
	const char* types_ptr;
	types_ptr = format;
	va_start(argp, format);
	bool nextSymbolIsAnArg = false;
	char currentCharacter;
	while (*types_ptr != '\0') 
	{
		currentCharacter = *types_ptr;
		if (nextSymbolIsAnArg)
		{
			nextSymbolIsAnArg = false;
			if (currentCharacter == 'i')
			{
				intArgument = va_arg(argp, int);
				sprintf(numberCharString, "%i", intArgument);
				length += StringLength(numberCharString) + 1;
			}
			else if (currentCharacter == 's')
			{
				stringArgument = va_arg(argp, char*);
				length += StringLength(stringArgument) + 1;
			}
			else if (currentCharacter == 'f')
			{
				floatArgument = va_arg(argp, float);
				sprintf(numberCharString, "%f", floatArgument);
				length += StringLength(numberCharString) + 1;
			}
			else if (currentCharacter == 'u')
			{
				uintArgument = va_arg(argp, uint32);
				sprintf(numberCharString, "%u", uintArgument);
				length += StringLength(numberCharString) + 1;
			}
			else if (currentCharacter == 'd')
			{
				uintArgument = va_arg(argp, uint32);
				sprintf(numberCharString, "%d", uintArgument);
				length += StringLength(numberCharString) + 1;
			}
			else
			{
				H_ASSERT(false, StringL::Format("Unsupported type, %s", currentCharacter));
			}
		}
		if (currentCharacter == '%')
		{
			nextSymbolIsAnArg = true;
		}

		++types_ptr;
	}

	va_end(argp);

	StringL str;
	va_list vl;
	va_start(vl, format);
	// allocate from length
	StringMemoryAllocator::GetInstance().AllocateString(nullptr, length, &str.m_memory.m_p);

	const int result = vsprintf_s(str.m_memory.m_p, length, format, vl);
	H_ASSERT(result > 0, "Invalid formatting.");
	va_end(vl);
	str.m_length = length;
	return str;
}

Hail::StringL::operator const char* () const
{
	return m_length > 15 ? m_memory.m_p : m_memory.m_shortString;
}

Hail::StringL::operator char* ()
{
	return m_length > 15 ? m_memory.m_p : m_memory.m_shortString;
}

StringL& Hail::StringL::operator=(const StringL& anotherString)
{
	if (m_length >= anotherString.Length())
	{
		if (m_length > 15)
		{
			memcpy(m_memory.m_p, anotherString.m_memory.m_p, anotherString.m_length);
			m_memory.m_p[anotherString.m_length] = 0;

		}
		else
			strcpy_s(m_memory.m_shortString, anotherString.m_memory.m_shortString);
		m_length = anotherString.Length();
		return *this;
	}
	else
	{
		if (m_length > 15)
			StringMemoryAllocator::GetInstance().DeallocateString(&m_memory.m_p);
	}
	m_length = anotherString.Length();
	if (anotherString.Length() > 15)
	{
		StringMemoryAllocator::GetInstance().AllocateString(anotherString.m_memory.m_p, m_length, &m_memory.m_p);
		memcpy(m_memory.m_p, anotherString.m_memory.m_p, anotherString.m_length);
		m_memory.m_p[anotherString.m_length] = 0;
	}
	else
	{
		strcpy_s(m_memory.m_shortString, anotherString.m_memory.m_shortString);
	}
	return *this;
}

StringL& Hail::StringL::operator=(StringL&& moveableString)
{
	if (m_length >= moveableString.Length())
	{
		if (m_length > 15)
		{
			memcpy(m_memory.m_p, moveableString.m_memory.m_p, moveableString.m_length);
			m_memory.m_p[moveableString.m_length] = 0;
		}
		else
			strcpy_s(m_memory.m_shortString, moveableString.m_memory.m_shortString);

		m_length = moveableString.Length();
		moveableString.m_memory.m_p = nullptr;
		moveableString.m_length = 0;
		return *this;
	}
	else
	{
		if (m_length > 15)
			StringMemoryAllocator::GetInstance().DeallocateString(&m_memory.m_p);
	}
	m_length = moveableString.Length();
	if (m_length > 15)
	{
		StringMemoryAllocator::GetInstance().AllocateString(moveableString.m_memory.m_p, m_length, &m_memory.m_p);
		memcpy(m_memory.m_p, moveableString.m_memory.m_p, moveableString.m_length);
		//m_memory.m_p[moveableString.m_length] = 0;
	}
	else
	{
		strcpy_s(m_memory.m_shortString, moveableString.m_memory.m_shortString);
	}
	moveableString.m_memory.m_p = nullptr;
	moveableString.m_length = 0;
	return *this;
}

StringL& Hail::StringL::operator=(const char* const pString)
{
	m_length = StringLength(pString);
	if (m_length > 15)
	{
		StringMemoryAllocator::GetInstance().AllocateString(pString, m_length, &m_memory.m_p);
	}
	else
	{
		strcpy_s(m_memory.m_shortString, pString);
	}
	return *this;
}

StringL& Hail::StringL::operator+=(StringL& anotherString)
{
	const uint32 previousLength = Length();
	m_length = previousLength + anotherString.Length();
	if (m_length > 15)
	{
		const char* previousString = m_memory.m_p;
		StringMemoryAllocator::GetInstance().AllocateString(nullptr, m_length, &m_memory.m_p);
		memcpy(m_memory.m_p, previousString, previousLength);
		if (anotherString.m_length > 15)
			memcpy(m_memory.m_p + (previousLength), anotherString.m_memory.m_p, anotherString.m_length);
		else
			memcpy(m_memory.m_p + (previousLength), anotherString.m_memory.m_shortString, anotherString.m_length);

		m_memory.m_p[m_length] = 0;
	}
	else
	{
		memcpy(&m_memory.m_shortString[previousLength], anotherString.m_memory.m_shortString, anotherString.Length());
		m_memory.m_shortString[m_length] = 0;
	}

	return *this;
}

StringL& Hail::StringL::operator+=(const char* pString)
{
	const uint32 previousLength = Length();
	const uint32 newStringLength = StringLength(pString);
	m_length = Length() + newStringLength;
	if (m_length > 15)
	{
		const char* previousString = m_memory.m_p;
		StringMemoryAllocator::GetInstance().AllocateString(nullptr, m_length, &m_memory.m_p);
		memcpy(m_memory.m_p, previousString, previousLength);
		memcpy(m_memory.m_p + (previousLength), pString, newStringLength);
		m_memory.m_p[m_length] = 0;
	}
	else
	{
		memcpy(&m_memory.m_shortString[previousLength], pString, newStringLength);
		m_memory.m_shortString[m_length] = 0;
	}

	return *this;
}

StringL Hail::StringL::operator+(const StringL& string1)
{
	const uint32 previousLength = m_length;
	m_length = string1.m_length + m_length;
	if (m_length > 15)
	{
		memcpy(&m_memory.m_shortString[previousLength], string1.m_memory.m_shortString, string1.m_length);
		m_memory.m_shortString[m_length] = 0;
	}
	else
	{
		char previousShortString[16];
		if (previousLength < 16)
			memcpy(previousShortString, m_memory.m_shortString, previousLength);

		const char* previousString = previousLength < 16 ? previousShortString : m_memory.m_p;
		StringMemoryAllocator::GetInstance().AllocateString(nullptr, m_length, &m_memory.m_p);
		memcpy(m_memory.m_p, previousString, previousLength);

		if (string1.m_length > 15)
			memcpy(m_memory.m_p + (previousLength), string1.m_memory.m_p, string1.m_length);
		else
			memcpy(m_memory.m_p + (previousLength), string1.m_memory.m_shortString, string1.m_length);

		m_memory.m_p[m_length] = 0;
	}
	return *this;
}

char* Hail::StringL::Data()
{
	return m_length > 16u ? m_memory.m_p : m_memory.m_shortString;
}

const char* const Hail::StringL::Data() const
{
	return m_length > 16u ? m_memory.m_p : m_memory.m_shortString;
}
