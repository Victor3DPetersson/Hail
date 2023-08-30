#pragma once
#include <string.h>
#include <string>
#include <assert.h>
#include <stdarg.h>
#include <clocale>
#include "String.hpp"

namespace Hail
{
	constexpr uint32_t MAX_FILE_LENGTH = 1024;
	class FilePath;
	struct FileTime
	{
		uint32_t m_lowDateTime = 0;
		uint32_t m_highDateTime = 0;
	};
	struct CommonFileData
	{
		FileTime m_creationTime;
		FileTime m_lastWriteTime;

		uint64_t m_filesizeInBytes = 0;
	};

#ifdef PLATFORM_WINDOWS

	constexpr const wchar_t* g_SourceSeparatorAndEnd = L"\\\0";
	constexpr const wchar_t g_End = L'\0';
	constexpr const wchar_t g_Wildcard = L'*';
	constexpr const wchar_t g_SourceSeparator = L'\\';
	constexpr const wchar_t g_Separator = L'/';




#endif


	class FileObject
	{
	public:
		FileObject();
		FileObject(const FileObject& otherObject);
		FileObject(const FilePath& filePath);
		FileObject(const wchar_t* const string, const FileObject& parentObject, CommonFileData fileData);
		FileObject(const char* const name, const char* extension, const FilePath& directoryLevel);

		FileObject& operator=(const FileObject& object);
		FileObject& operator=(const FileObject&& moveableObject) noexcept;
		const FileObject& operator=(FileObject& object);
		const FileObject& operator=(FileObject&& moveableObject) noexcept;

		const WString64& Name() const { return m_name; }
		const WString64& ParentName() const { return m_parentName; }
		const WString64& Extension() const { return m_extension; }
		bool IsDirectory() const { return m_isDirectory; }
		bool IsValid() const;
		bool IsFile() const { return !m_isDirectory; }
		uint32_t Length() const { return m_name.Length(); }
		const CommonFileData& GetFileData() const { return m_fileData; }
		uint64_t GetLastWriteFileTime() const;
		uint16_t GetDirectoryLevel() const { return m_directoryLevel; }
	private:
		void Reset();
		bool FindExtension();

		WString64 m_name;
		WString64 m_parentName;
		WString64 m_extension;
		CommonFileData m_fileData;
		bool m_isDirectory;
		uint16_t m_directoryLevel;
	};

	class FilePath
	{
	public:
		FilePath()
		{
			Reset();
		}
		FilePath(const char* const string)
		{
			Reset();
			size_t length = strlen(string);
			if (length < MAX_FILE_LENGTH && length != 0)
			{
				mbstowcs(m_data, string, length + 1);
				m_length = length;
				FindExtension();
			}
			else
			{
				//error
			}
		}
		FilePath(const wchar_t* const string)
		{
			Reset();
			size_t length = wcslen(string);
			if (length < MAX_FILE_LENGTH)
			{
				wcscpy_s(m_data, string);
				m_length = length;
				FindExtension();
			}
			else
			{
				//error
			}
		}
		FilePath(const FilePath& path)
		{
			wcscpy_s(m_data, path.m_data);
			m_object = path.m_object;
			m_length = path.m_length;
			m_isDirectory = path.m_isDirectory;
			m_directoryLevel = path.m_directoryLevel;
		}
		FilePath(FilePath&& moveablePath) noexcept
		{
			wcscpy_s(m_data, moveablePath);
			m_object = moveablePath.m_object;
			m_length = moveablePath.m_length;
			m_isDirectory = moveablePath.m_isDirectory;
			m_directoryLevel = moveablePath.m_directoryLevel;
		}
		operator const wchar_t* () const
		{
			return m_data;
		}
		operator wchar_t* ()
		{
			return m_data;
		}
		FilePath& operator=(const FilePath& path)
		{
			wcscpy_s(m_data, path.m_data);
			m_object = path.m_object;
			m_length = path.m_length;
			m_isDirectory = path.m_isDirectory;
			m_directoryLevel = path.m_directoryLevel;
			return *this;
		}
		FilePath& operator=(FilePath&& moveablePath) noexcept
		{
			wcscpy_s(m_data, moveablePath.m_data);
			m_object = moveablePath.m_object;
			m_length = moveablePath.m_length;
			m_isDirectory = moveablePath.m_isDirectory;
			m_directoryLevel = moveablePath.m_directoryLevel;
			return *this;
		}
		FilePath& operator=(const wchar_t* const string)
		{
			Reset();
			wcscpy_s(m_data, string);
			m_length = wcslen(string);
			FindExtension();
			return *this;
		}

		FilePath operator+(const FilePath& otherPath);
		FilePath operator+(const FileObject& object);
		FilePath operator+(const wchar_t* const string);
		FilePath Parent() const;
		//Either the file object or the path
		const FileObject& Object() const { return m_object; }
		const wchar_t* Data() const	{ return m_data; }

		void Slashify();

		bool IsDirectory() const { return m_isDirectory; }
		bool IsValid() const;
		bool IsFile() const	{ return !m_isDirectory; }
		uint32_t Length() const { return m_length; }
		uint16_t GetDirectoryLevel() const { return m_directoryLevel; }
		void AddWildcard();
		//Creates a directory if none exist, otherwise this function does nothing 
		bool CreateFileDirectory() const;
	protected:
		FilePath(const FilePath& path, uint32_t lengthOfPath);
		void FindExtension();
		void CreateFileObject();

	private:
		void Reset();
		wchar_t m_data[MAX_FILE_LENGTH];
		FileObject m_object;
		uint32_t m_length = 0;
		bool m_isDirectory = false;
		uint16_t m_directoryLevel = 0;
	};
}

namespace Hail
{
	static bool operator<(const FilePath& path1, const FilePath& path2)
	{
		return path1.Length() < path2.Length();
	}
	static bool operator>(const FilePath& path1, const FilePath& path2)
	{
		return path1.Length() > path2.Length();
	}
	static bool operator==(const FilePath& path1, const FilePath& path2)
	{
		if (path1.Length() == path2.Length() && 
			path1.IsDirectory() == path2.IsDirectory())
		{
			return wcscmp(path1.Data(), path2.Data()) == 0;
		}
		return false;
	}
	static bool operator!=(const FilePath& path1, const FilePath& path2)
	{
		if (path1.Length() == path2.Length() &&
			path1.IsDirectory() == path2.IsDirectory())
		{
			return wcscmp(path1.Data(), path2.Data()) != 0;
		}
		return true;
	}
	//static FilePath operator+(const FilePath& string1, const FilePath& string2)
	//{
	//	FilePath path = string1;
	//	if (string1.IsDirectory())
	//	{
	//		if (path.Data()[path.Length() - 1] == L'*')
	//		{
	//			path.DataRef()[path.Length() - 1] = L'\0';
	//			wcscat_s(path, MAX_FILE_LENGTH - string1.Length(), string2.Data());
	//		}
	//		else
	//		{
	//			//wcscat_s(path, MAX_FILE_LENGTH - string1.Length(), string2.Data());
	//			memcpy(path.DataRef() + string1.Length(), string2.Data(), string2.Length());
	//		}
	//	}
	//	path.FindExtension();
	//	return path;
	//}
}
