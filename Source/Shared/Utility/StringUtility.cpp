#include "Shared_PCH.h"
#include "StringUtility.h"
#include <cstdlib>
#include <stdlib.h>

namespace Hail
{

	size_t StringLength(const char* string)
	{
		size_t currentCharacterCount = 0;
		char currentChar = string[currentCharacterCount];
		while (currentChar != '\0')
		{
			currentChar = string[++currentCharacterCount];
		}
		return currentCharacterCount;
	}

	size_t StringLength(const wchar_t* string)
	{
		size_t currentCharacterCount = 0;
		wchar_t currentChar = string[currentCharacterCount];
		while (currentChar != '\0')
		{
			currentChar = string[++currentCharacterCount];
		}
		return currentCharacterCount;
	}

	bool StringCompare(const char* string1, const char* string2)
	{
		const size_t size1 = StringLength(string1);
		const size_t size2 = StringLength(string2);
		if (size1 != size2)
			return false;

		for (size_t i = 0; i < size1; i++)
		{
			if (string1[i] != string2[i])
			{
				return false;
			}
		}
		return true;
	}
	bool StringCompareCaseInsensitive(const char* string1, const char* string2)
	{
		const size_t size1 = StringLength(string1);
		const size_t size2 = StringLength(string2);
		if (size1 != size2)
			return false;

		for (size_t i = 0; i < size1; i++)
		{
			if (std::tolower(string1[i]) != std::tolower(string2[i]))
			{
				return false;
			}
		}
		return true;
	}
	bool StringCompare(const wchar_t* string1, const wchar_t* string2)
	{
		const size_t size1 = StringLength(string1);
		const size_t size2 = StringLength(string2);
		if (size1 != size2)
		{
			return false;
		}
		for (size_t i = 0; i < size1; i++)
		{
			if (string1[i] != string2[i])
			{
				return false;
			}
		}
		return true;
	}
	bool StringContains(const char* string, const char charToFind)
	{
		const size_t stringSize = StringLength(string);
		char currentCharacter = string[0];
		for (size_t i = 0; i < stringSize;)
		{
			if (currentCharacter == charToFind)
			{
				return true;
			}
			currentCharacter = string[++i];
		}
		return false;
	}

	int StringContainsAndEndsAtIndex(const char* string, const char* stringToFind)
	{
		const size_t stringSize = StringLength(string);
		const size_t stringToFindSize = StringLength(stringToFind);

		if (stringToFindSize > stringSize)
		{
			return INVALID_STRING_RESULT;
		}
		char currentCharacter = string[0];
		for (size_t i = 0; i < stringSize;)
		{
			//if first character is same as current character
			char characterInStringToFind = stringToFind[0];
			if (currentCharacter == characterInStringToFind)
			{
				if (stringToFindSize > stringSize - i)
				{
					return INVALID_STRING_RESULT;
				}
				bool containsString = true;
				for (size_t j = 1; j < stringToFindSize; j++)
				{
					currentCharacter = string[i + j];
					characterInStringToFind = stringToFind[j];
					if (currentCharacter != characterInStringToFind)
					{
						containsString = false;
						break;
					}
				}
				if (containsString)
				{
					return i;
				}
			}
			else
			{
				if (stringToFindSize > stringSize - i)
				{
					return INVALID_STRING_RESULT;
				}
			}
			currentCharacter = string[++i];
		}
		return INVALID_STRING_RESULT;
	}

	bool StringContains(const char* string, const char* stringToFind)
	{
		return StringContainsAndEndsAtIndex(string, stringToFind) != INVALID_STRING_RESULT;
	}

	//This function will return the location of where the token was found or that it reached end of string
	int LengthUntilEndOrToken(const char* stringToSearchIn, int startToken, const char characterToSearchFor)
	{
		const size_t stringSize = StringLength(stringToSearchIn);
		//Overflow check
		if (startToken >= stringSize)
		{
			return -1;
		}
		int currentToken = startToken;
		char currentCharacter = stringToSearchIn[currentToken];
		while (currentToken < stringSize)
		{
			if (currentCharacter == characterToSearchFor)
			{
				return currentToken - startToken;
			}
			currentCharacter = stringToSearchIn[++currentToken];
		}
		return END_OF_STRING;
	}

	void AddToString(char* stringLiteralToAddToo, const char* stringLiteralToAdd, size_t stringCapacity)
	{
		const size_t stringSize = StringLength(stringLiteralToAddToo);
		const size_t stringToAddSize = StringLength(stringLiteralToAdd);
		if (stringSize + stringToAddSize >= stringCapacity)
		{
			return;
		}
		for (size_t i = stringSize, charToAddIndex = 0; i < stringSize + stringToAddSize; i++, charToAddIndex++)
		{
			stringLiteralToAddToo[i] = stringLiteralToAdd[charToAddIndex];
		}
		stringLiteralToAddToo[stringSize + stringToAddSize] = '\0';
	}

	void FromWCharToConstChar(const wchar_t* inWChar, char* outChar, uint32 outCharMaxlength)
	{
		size_t inWCharLength = StringLength(inWChar);
		if (inWCharLength >= outCharMaxlength)
		{
			wcstombs(outChar, inWChar, outCharMaxlength);
			outChar[outCharMaxlength] = '\0';
		}
		else
		{
			wcstombs(outChar, inWChar, inWCharLength + 1);
		}
	}

	void FromConstCharToWChar(const char* inChar, wchar_t* outChar, size_t outCharMaxlength)
	{
		size_t inCharLength = StringLength(inChar);
		if (inCharLength >= outCharMaxlength)
		{
			mbstowcs(outChar, inChar, outCharMaxlength);
			outChar[outCharMaxlength] = '\0';
		}
		else
		{
			mbstowcs(outChar, inChar, inCharLength + 1);
		}
	}

	int32 Hail::StringUtility::IntFromConstChar(const char* charToRead, uint32 startPosition)
	{
		return strtol(charToRead + startPosition, nullptr, 10);
	}

	int32 StringUtility::FindFirstOfSymbol(const char* charToRead, char symbolToFind)
	{
		const size_t length = StringLength(charToRead);
		for (size_t i = 0; i < length; i++)
		{
			if (charToRead[i] == symbolToFind)
				return (int32)i;
		}
		return -1;
	}
}


