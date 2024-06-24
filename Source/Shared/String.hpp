#pragma once
#include <string.h>
#include <string>
#include <assert.h>
#include <stdarg.h>

#include "Utility/StringUtility.h"

//TODO: namespace Hail this file

class String64
{
public:
	String64()
	{
		strcpy_s(m_data, "");
	}
	String64(const char* const aString)
	{
		strcpy_s(m_data, aString);
	}
	String64(const String64& anotherString)
	{
		strcpy_s(m_data, anotherString);
	}
	String64(String64&& aMoveableString) noexcept
	{
		strcpy_s(m_data, aMoveableString);
	}
	String64(const std::string& aSTLString)
	{
		assert(aSTLString.size() < 64);
		if (aSTLString.size() < 64)
		{
			strcpy_s(m_data, aSTLString.data());
		}
		else
		{
			strcpy_s(m_data, "errors n stuff");
		}
	}
	static String64 Format(const char* const aFormat, ...)
	{
		String64 str;
		va_list vl;
		va_start(vl, aFormat);
		vsprintf_s(str.m_data, aFormat, vl);
		va_end(vl);
		return str;
	}

	operator const char* () const
	{
		return m_data;
	}
	operator char* ()
	{
		return m_data;
	}
	String64& operator=(const String64& anotherString)
	{
		strcpy_s(m_data, anotherString);

		return *this;
	}
	String64& operator=(String64&& aMoveableString) noexcept
	{
		strcpy_s(m_data, aMoveableString);

		return *this;
	}
	String64& operator=(const char* const aCString)
	{
		strcpy_s(m_data, aCString);

		return *this;
	}
	char* Data()
	{
		return m_data;
	}
	const char* const Data() const
	{
		return m_data;
	}

	bool Empty() const
	{
		return m_data[0] == '\0';
	}
	const uint32_t Length() const { return strlen(m_data); }

private:
	char m_data[64];
};

static bool operator<(const String64& aString1, const String64& aString2)
{
	return strcmp(aString1, aString2) < -0;
}
static bool operator>(const String64& aString1, const String64& aString2)
{
	return strcmp(aString1, aString2) > 0;
}
static bool operator==(const String64& aString1, const String64& aString2)
{
	return strcmp(aString1, aString2) == 0;
}
static bool operator!=(const String64& aString1, const String64& aString2)
{
	return strcmp(aString1, aString2) != 0;
}
static String64 operator+(const String64& aString1, const String64& aString2)
{
	String64 str = aString1;
	strcat_s(str, 64 - strlen(aString1), aString2);
	return str;
}

class WString64
{
public:
	WString64()
	{
		wcscpy_s(m_data, L"");
	}
	WString64(const wchar_t* const aString)
	{
		wcscpy_s(m_data, aString);
	}
	WString64(const WString64& anotherString)
	{
		wcscpy_s(m_data, anotherString);
	}
	WString64(WString64&& aMoveableString) noexcept
	{
		wcscpy_s(m_data, aMoveableString);
	}
	WString64(const std::wstring& aSTLString)
	{
		assert(aSTLString.size() < 64);
		if (aSTLString.size() < 64)
		{
			wcscpy_s(m_data, aSTLString.data());
		}
		else
		{
			wcscpy_s(m_data, L"errors n stuff");
		}
	}
	static WString64 Format(const wchar_t* const aFormat, ...)
	{
		WString64 str;
		va_list vl;
		va_start(vl, aFormat);
		vswprintf_s(str.m_data, aFormat, vl);
		va_end(vl);
		return str;
	}

	operator const wchar_t* () const 
	{
		return m_data;
	}

	operator wchar_t* ()
	{
		return m_data;
	}
	WString64& operator=(const WString64& anotherString)
	{
		wcscpy_s(m_data, anotherString);

		return *this;
	}
	WString64& operator=(WString64&& aMoveableString) noexcept
	{
		wcscpy_s(m_data, aMoveableString);

		return *this;
	}
	WString64& operator=(const wchar_t* const aCString)
	{
		wcscpy_s(m_data, aCString);

		return *this;
	}
	wchar_t* Data()
	{
		return m_data;
	}
	const uint32_t Length() const { return wcslen(m_data); }

	String64 CharString() const 
	{ 
		String64 returnString;
		Hail::FromWCharToConstChar(m_data, returnString.Data(), 64u);
		return returnString;
	}

private:
	wchar_t m_data[64];
};

static bool operator<(const WString64& aString1, const WString64& aString2)
{
	return wcscmp(aString1, aString2) < -0;
}
static bool operator>(const WString64& aString1, const WString64& aString2)
{
	return wcscmp(aString1, aString2) > 0;
}
static bool operator==(const WString64& aString1, const WString64& aString2)
{
	return wcscmp(aString1, aString2) == 0;
}
static bool operator!=(const WString64& aString1, const WString64& aString2)
{
	return wcscmp(aString1, aString2) != 0;
}
static WString64 operator+(const WString64& aString1, const WString64& aString2)
{
	WString64 str = aString1;
	wcscat_s(str, 64 - wcslen(aString1), aString2);
	return str;
}

class String256
{
public:
	String256()
	{
		strcpy_s(m_data, "");
	}
	String256(const char* const aString)
	{
		strcpy_s(m_data, aString);
	}
	String256(const String256& anotherString)
	{
		strcpy_s(m_data, anotherString);
	}
	String256(String256&& aMoveableString) noexcept
	{
		strcpy_s(m_data, aMoveableString);
	}
	String256(const std::string& aSTLString)
	{
		assert(aSTLString.size() < 256);
		if (aSTLString.size() < 256)
		{
			strcpy_s(m_data, aSTLString.data());
		}
		else
		{
			strcpy_s(m_data, "errors n stuff");
		}
	}
	static String256 Format(const char* const aFormat, ...)
	{
		String256 str;
		va_list vl;
		va_start(vl, aFormat);
		vsprintf_s(str.m_data, aFormat, vl);
		va_end(vl);
		return str;
	}

	operator const char* () const
	{
		return m_data;
	}
	operator char* ()
	{
		return m_data;
	}
	String256& operator=(const String256& anotherString)
	{
		strcpy_s(m_data, anotherString);

		return *this;
	}
	String256& operator=(String256&& aMoveableString) noexcept
	{
		strcpy_s(m_data, aMoveableString);

		return *this;
	}
	String256& operator=(const char* const aCString)
	{
		strcpy_s(m_data, aCString);

		return *this;
	}
	String256& operator+=(String256& anotherString) noexcept
	{
		const int stringLength = anotherString.Length();
		const uint32_t existingLength = Length();

		const int remainingLength = 256 - existingLength;
		const int stringLengthToCopy = stringLength > remainingLength ? remainingLength : stringLength;

		memcpy(&m_data[existingLength], anotherString, stringLengthToCopy);
		m_data[existingLength + stringLengthToCopy] = 0;
		return *this;
	}
	String256& operator+=(const char* cString) noexcept
	{
		const int stringLength = Hail::StringLength(cString);
		const uint32_t existingLength = Length();

		const int remainingLength = 256 - existingLength;
		const int stringLengthToCopy = stringLength > remainingLength ? remainingLength : stringLength;

		memcpy(&m_data[existingLength], cString, stringLengthToCopy);
		m_data[existingLength + stringLengthToCopy] = 0;
		return *this;
	}
	char* Data()
	{
		return m_data;
	}
	const char* const Data() const
	{
		return m_data;
	}

	bool Empty() const
	{
		return m_data[0] == '\0';
	}
	const uint32_t Length() const { return strlen(m_data); }

private:
	char m_data[256];
};

static bool operator<(const String256& aString1, const String256& aString2)
{
	return strcmp(aString1, aString2) < -0;
}
static bool operator>(const String256& aString1, const String256& aString2)
{
	return strcmp(aString1, aString2) > 0;
}
static bool operator==(const String256& aString1, const String256& aString2)
{
	return strcmp(aString1, aString2) == 0;
}
static bool operator!=(const String256& aString1, const String256& aString2)
{
	return strcmp(aString1, aString2) != 0;
}
static String256 operator+(const String256& aString1, const String256& aString2)
{
	String256 str = aString1;
	strcat_s(str, 256 - strlen(aString1), aString2);
	return str;
}

class WString256
{
public:
	WString256()
	{
		wcscpy_s(m_data, L"");
	}
	WString256(const wchar_t* const aString)
	{
		wcscpy_s(m_data, aString);
	}
	WString256(const WString256& anotherString)
	{
		wcscpy_s(m_data, anotherString);
	}
	WString256(WString256&& aMoveableString) noexcept
	{
		wcscpy_s(m_data, aMoveableString);
	}
	WString256(const std::wstring& aSTLString)
	{
		assert(aSTLString.size() < 256);
		if (aSTLString.size() < 256)
		{
			wcscpy_s(m_data, aSTLString.data());
		}
		else
		{
			wcscpy_s(m_data, L"errors n stuff");
		}
	}
	static WString256 Format(const wchar_t* const aFormat, ...)
	{
		WString256 str;
		va_list vl;
		va_start(vl, aFormat);
		vswprintf_s(str.m_data, aFormat, vl);
		va_end(vl);
		return str;
	}

	operator const wchar_t* () const
	{
		return m_data;
	}

	operator wchar_t* ()
	{
		return m_data;
	}
	WString256& operator=(const WString256& anotherString)
	{
		wcscpy_s(m_data, anotherString);

		return *this;
	}
	WString256& operator=(WString256&& aMoveableString) noexcept
	{
		wcscpy_s(m_data, aMoveableString);

		return *this;
	}
	WString256& operator=(const wchar_t* const aCString)
	{
		wcscpy_s(m_data, aCString);

		return *this;
	}
	wchar_t* Data()
	{
		return m_data;
	}
	const uint32_t Length() const { return wcslen(m_data); }

private:
	wchar_t m_data[256];
};

static bool operator<(const WString256& string1, const WString256& string2)
{
	return wcscmp(string1, string2) < -0;
}
static bool operator>(const WString256& string1, const WString256& string2)
{
	return wcscmp(string1, string2) > 0;
}
static bool operator==(const WString256& string1, const WString256& string2)
{
	return wcscmp(string1, string2) == 0;
}
static bool operator!=(const WString256& string1, const WString256& string2)
{
	return wcscmp(string1, string2) != 0;
}
static WString256 operator+(const WString256& string1, const WString256& string2)
{
	WString256 str = string1;
	wcscat_s(str, 256 - wcslen(string1), string2);
	return str;
}