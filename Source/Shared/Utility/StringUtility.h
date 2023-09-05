#pragma once
#include "Types.h"

namespace Hail
{
	int constexpr END_OF_STRING = -2;
	int constexpr INVALID_STRING_RESULT = -1;

	size_t StringLength(const char* string);
	size_t StringLength(const wchar_t* string);

	bool StringCompare(const char* string1, const char* string2);
	bool StringCompare(const wchar_t* string1, const wchar_t* string2);

	bool StringContains(const char* string, const char charToFind);

	int StringContainsAndEndsAtIndex(const char* string, const char* stringToFind);

	bool StringContains(const char* string, const char* stringToFind);

	//This function will return the location of where the token was found or that it reached end of string
	int LengthUntilEndOrToken(const char* stringToSearchIn, int startToken, const char characterToSearchFor);

	void AddToString(char* stringLiteralToAddToo, const char* stringLiteralToAdd, size_t stringCapacity);

	void FromWCharToConstChar(const wchar_t* inWChar, char* outChar, uint32 outCharMaxlength);
	void FromConstCharToWChar(const char* inChar, wchar_t* outChar, size_t outCharMaxlength);
}
