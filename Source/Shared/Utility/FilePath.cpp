#include "Shared_PCH.h"
#include "FilePath.hpp"

#include <string>
#include <iostream>
#include "DebugMacros.h"

#ifdef PLATFORM_WINDOWS

#include <windows.h>
//#elif PLATFORM_OSX//.... more to be added

#endif

#include "Utility\StringUtility.h"

namespace
{
    int GetParentSeperator(const wchar_t* path, uint32_t length, bool isDirectory)
    {
        if (length == 0)
        {
            return 0;
        }
        int32_t seperator = length - 1;
        wchar_t currentCharacter = 0;
        bool foundFirstSeperator = false;
        while (seperator != -1)
        {
            if (isDirectory && !foundFirstSeperator)
            {
                if (currentCharacter == Hail::g_SourceSeparator)
                {
                    currentCharacter = L'\0';
                    foundFirstSeperator = true;
                }
            }
            if (currentCharacter == Hail::g_SourceSeparator)
            {
                return length - (seperator + 1);
                break;
            }
            currentCharacter = path[seperator--];
        }
        return -1;
    }
}

void Hail::FilePath::CreateFileObject()
{
    m_object = FileObject(*this);
}

void Hail::FilePath::AddWildcard()
{
    if (m_isDirectory && m_data[m_length - 1] != g_Wildcard)
    {
        m_data[m_length++] = g_Wildcard;
        m_data[m_length] = L'\0';
    }
}

bool Hail::FilePath::CreateFileDirectory() const
{
    FilePath pathToCreate = *this;
    if (!m_isDirectory)
    {
        pathToCreate = Parent();
    }
    if (!pathToCreate.IsValid())
    {
        uint16_t directoryLevelWhereParentExist = 0;
        {
            Hail::FilePath parentPath = pathToCreate.Parent();
            while (!parentPath.IsValid())
            {
                parentPath = parentPath.Parent();
                //There is no valid parent path, meaning the directory is entirely invalid
                if (parentPath.GetDirectoryLevel() == 1 && !parentPath.IsValid())
                {
                    return false;
                }
            }
            directoryLevelWhereParentExist = parentPath.GetDirectoryLevel();
        }

        //construct the entire folder hierarchy
        for (size_t i = pathToCreate.GetDirectoryLevel() - directoryLevelWhereParentExist; i > 0; i--)
        {
            pathToCreate = *this;
            while (pathToCreate.GetDirectoryLevel() != directoryLevelWhereParentExist + i)
            {
                pathToCreate = pathToCreate.Parent();
            }
#ifdef PLATFORM_WINDOWS
            if (CreateDirectory(pathToCreate.Data(), NULL) ||
                ERROR_ALREADY_EXISTS == GetLastError())
            {
                directoryLevelWhereParentExist++;
            }
            else
            {
                // Failed to create directory.
                std::cout << " Failed to create Directory! " << std::endl;
            }
#endif
        }
    }

    return true;
}

void Hail::FilePath::DeleteEndSeperator()
{
    if (!m_isDirectory)
        return;
    const wchar_t lastDigit = m_data[m_length - 1];
    if (lastDigit == g_SourceSeparator)
        m_data[--m_length] = L'\0';
}

Hail::FilePath::FilePath(const FilePath& path, uint32_t lengthOfPath)
{
    *this = path;
    if (m_directoryLevel == 1)
        --lengthOfPath;

    for (; ; --m_length)
    {
        m_data[m_length] = L'\0';
        lengthOfPath--;
        if (lengthOfPath == 0)
        {
            break;
        }
    }
    FindExtension();
}

Hail::FilePath Hail::FilePath::operator+(const FilePath& otherPath)
{
    if (otherPath.IsValid())
    {
        if (m_isDirectory)
        {
            if (m_data[m_length - 1] == g_Wildcard)
            {
                m_data[--m_length] = L'\0';
                wcscat_s(m_data, Hail::MAX_FILE_LENGTH - m_length, otherPath.Data());
                m_length += otherPath.Length();
            }
            else
            {
                //memcpy(m_data + m_length, otherPath.Data(), otherPath.Length());
                wcscat_s(m_data, MAX_FILE_LENGTH - m_length, otherPath.Data());
            }
        }
        else
        {
            wcscat_s(m_data, MAX_FILE_LENGTH - m_length, otherPath.Data());
        }
        FindExtension();
    }
    return *this;
}

Hail::FilePath Hail::FilePath::operator+(const FileObject& object) const
{
    FilePath returnPath = *this;
    uint32_t length = 0;
    if (object.IsValid() && m_isDirectory)
    {
        if (returnPath.m_length != 0 && m_data[returnPath.m_length - 1] == g_Wildcard)
        {
            returnPath.m_data[--returnPath.m_length] = L'\0';
        }
        length += object.Name().Length();
        wcscat_s(returnPath.m_data, MAX_FILE_LENGTH - returnPath.m_length, object.Name());
        if (object.IsDirectory())
        {
            returnPath.m_data[returnPath.m_length + length++] = g_SourceSeparator;
        }
        else
        {
            returnPath.m_data[returnPath.m_length + length++] = L'.';
            returnPath.m_data[returnPath.m_length + length] = g_End;
            length += object.Extension().Length();
            wcscat_s(returnPath.m_data, MAX_FILE_LENGTH - (returnPath.m_length + length), object.Extension());
        }
        returnPath.m_data[returnPath.m_length + length] = g_End;
        returnPath.m_length += length;
        returnPath.FindExtension();
    }
    returnPath.m_object = object;
    return returnPath;
}

Hail::FilePath Hail::FilePath::operator+(const wchar_t* const string)
{
    uint32_t length = wcslen(string);
    if (length > 0)
    {
        if (m_isDirectory)
        {
            if (m_data[m_length - 1] == g_Wildcard)
            {
                m_data[--m_length] = L'\0';
                wcscat_s(m_data, Hail::MAX_FILE_LENGTH - m_length, string);
            }
            else
            {
                wcscat_s(m_data, MAX_FILE_LENGTH - m_length, string);
            }
        }
        else
        {
            wcscat_s(m_data, MAX_FILE_LENGTH - m_length, string);
        }
        m_length += length;
        FindExtension();
    }
    return *this;
}

Hail::FilePath Hail::FilePath::Parent() const
{
    if (m_directoryLevel >= 1)
    {
        const int objectLength = GetParentSeperator(m_data, m_length, m_isDirectory);
        FilePath path = FilePath(*this, objectLength + 1);
        return path;
    }
    else
    {
        //No parent directory
        return *this;
    }
}


void Hail::FilePath::Slashify()
{
    size_t size = StringLength(m_data);
    if (size == 0)
    {
        return;
    }
    if (m_data[size - 1] == g_SourceSeparator || m_data[size - 1] == g_Separator)
    {
        m_data[size - 1] = g_End;
        size--;
    }

    m_directoryLevel = 0;
    wchar_t temp[MAX_FILE_LENGTH];
    temp[0] = L'\0';
    wcscpy_s(temp, m_data);
    uint32_t counter = 0;
    size_t i(0);
    for (; i < size; i++)
    {
        if ((temp[i] == g_SourceSeparator || temp[i] == g_Separator) && temp[i + 1] != g_SourceSeparator && temp[i + 1] != g_Separator)
        {
            m_data[counter++] = g_SourceSeparator;
            m_directoryLevel++;
        }
        else if (m_data[i] != g_SourceSeparator && m_data[i] != g_Separator)
        {
            m_data[counter++] = temp[i];
        }
    }

    if (m_isDirectory)
    {
        if (m_data[counter - 1] != g_SourceSeparator && m_data[counter - 1] != L'*')
        {
            m_data[counter] = g_SourceSeparator;
            m_data[++counter] = L'\0';
        }
    }
    m_length = counter;
}

bool Hail::FilePath::IsValid() const
{
    if (m_length == 0)
    {
        return false;
    }
    return IsValidFilePathInternal(this);
}

void Hail::FilePath::FindExtension()
{
    if (m_length == 0)
    {
        return;
    }
    m_isDirectory = true;
    int32_t seperator = m_length - 1;
    wchar_t currentCharacter = 0;
    while (seperator != 0)
    {
        if (currentCharacter == g_Wildcard)
        {
            break;
        }
        if (currentCharacter == L'.')
        {
            m_isDirectory = false;
            break;
        }
        currentCharacter = m_data[seperator--];
    }
    Slashify();
    CreateFileObject();
}

void Hail::FileObject::SetExtension(WString64 newExtension)
{
    if (!m_isDirectory)
    {
        m_extension = newExtension;
    }
}

bool Hail::FileObject::IsValid() const
{
    return m_name.Length() != 0;
}

void Hail::FilePath::Reset()
{
    m_data[0] = L'\0';
    m_length = 0;
    m_isDirectory = true;
    m_directoryLevel = 0;
}

//Hail::FilePath operator+(const Hail::FilePath& string1, const Hail::FilePath& string2)
//{
//    Hail::FilePath path = string1;
//    if (string1.IsDirectory())
//    {
//        if (path.Data()[path.Length() - 1] == L'*')
//        {
//            path.DataRef()[path.Length() - 1] = L'\0';
//            wcscat_s(path, Hail::MAX_FILE_LENGTH - string1.Length(), string2.Data());
//        }
//        else
//        {
//            //wcscat_s(path, MAX_FILE_LENGTH - string1.Length(), string2.Data());
//            memcpy(path.DataRef() + string1.Length(), string2.Data(), string2.Length());
//        }
//    }
//    path.FindExtension();
//    return path;
//}

Hail::FileObject::FileObject()
{
    Reset();
}

Hail::FileObject::FileObject(const FileObject& otherObject) : 
    m_name(otherObject.m_name),
    m_parentName(otherObject.m_parentName)
{
    m_extension = otherObject.m_extension;
    m_isDirectory = otherObject.m_isDirectory;
    m_fileData = otherObject.m_fileData;
    m_directoryLevel = otherObject.m_directoryLevel;
}

Hail::FileObject::FileObject(const FilePath& filePath)
{
    Reset();
    if(filePath.Length() == 0)
    {
        return;
    }

    if (filePath.GetDirectoryLevel() < 1)
    {
        m_name.Data()[0] = filePath.Data()[0];
        m_name.Data()[1] = L':';
        m_name.Data()[2] = g_End;
        m_directoryLevel = 0;
        m_isDirectory = true;
        const wchar_t* folderName = L"folder\0";
        memcpy(m_extension, folderName, sizeof(wchar_t) * 7);
        return;
    }

    const int32_t parentSeperator = GetParentSeperator(filePath.Data(), filePath.Length(), filePath.IsDirectory());
    m_name = filePath.Data() + (filePath.Length() - parentSeperator) + 1;
    const int32_t length = m_name.Length();
    if (filePath.IsDirectory())
    {
        if (filePath.Data()[filePath.Length() - 1] == g_Wildcard)
        {
            m_name.Data()[length - 2] = g_End;
            //m_length--;
        }
        m_name.Data()[length - 1] = g_End;
        m_isDirectory = true;
        const wchar_t* folderName = L"folder\0";
        memcpy(m_extension, folderName, sizeof(wchar_t) * 7);
        //m_data[--m_length] = L'\0';
    }
    else
    {
        int32_t seperator = m_name.Length();
        wchar_t currentCharacter = 0;
        while (seperator != 0)
        {
            if (currentCharacter == g_Wildcard)
            {
                break;
            }
            if (currentCharacter == L'.')
            {
                m_isDirectory = false;
                seperator++;
                break;
            }
            currentCharacter = m_name[seperator--];
        }
        const uint32_t fileExtensionLength = m_name.Length() != 0 ? m_name.Length() - (seperator + 1) : 0;
        memcpy(m_extension, m_name + seperator + 1, sizeof(wchar_t) * fileExtensionLength);
        m_extension[fileExtensionLength] = g_End;
        memset(m_name + seperator, 0, sizeof(wchar_t) * (fileExtensionLength + 1));
    }

    int32_t parentParentSeperator = GetParentSeperator(filePath.Data(), (filePath.Length() - parentSeperator) + 1, true);
    if (parentParentSeperator != -1)
    {
        memcpy(m_parentName, filePath.Data() + ((filePath.Length() - parentSeperator) - parentParentSeperator) + 2, sizeof(wchar_t) * (parentParentSeperator));
        m_parentName[parentParentSeperator - 2] = g_End;
    }
    //Parent is Root directory
    else
    {
        m_parentName[0] = filePath.Data()[0];
        m_parentName[1] = L'\0';
    }

    m_directoryLevel = filePath.GetDirectoryLevel();
    m_fileData = ConstructFileDataFromPath(filePath);
}

Hail::FileObject::FileObject(const wchar_t* const string, const FileObject& parentObject, CommonFileData fileData) :
    m_fileData(fileData)
{
    Reset();
    m_fileData = fileData;
    m_name = string;
    m_parentName = parentObject.Name();
    m_directoryLevel = parentObject.GetDirectoryLevel() + 1;
    FindExtension();
}

Hail::FileObject::FileObject(const char* const name, const char* extension, const FilePath& directoryLevel)
{
    FileObject parentObject;
    m_directoryLevel = directoryLevel.GetDirectoryLevel();
    if (directoryLevel.IsDirectory())
    {
        m_directoryLevel++;
        parentObject = directoryLevel.Object();
    }
    else
    {
        parentObject = directoryLevel.Parent().Object();
    }
    memcpy(m_parentName, parentObject.m_name, sizeof(m_parentName));

    FromConstCharToWChar(name, m_name, 64);
    FromConstCharToWChar(extension, m_extension, 64);
    m_isDirectory = false;

}

Hail::FileObject& Hail::FileObject::operator=(const FileObject& object)
{
    m_name = object.m_name;
    m_parentName = object.m_parentName;
    m_extension = object.m_extension;
    m_isDirectory = object.m_isDirectory;
    m_fileData = object.m_fileData;
    m_directoryLevel = object.m_directoryLevel;
    return *this;
}

Hail::FileObject& Hail::FileObject::operator=(const FileObject&& moveableObject) noexcept
{
    m_name = moveableObject.m_name;
    m_parentName = moveableObject.m_parentName;
    m_extension = moveableObject.m_extension;
    m_isDirectory = moveableObject.m_isDirectory;
    m_fileData = moveableObject.m_fileData;
    m_directoryLevel = moveableObject.m_directoryLevel;
    return *this;
}

const Hail::FileObject& Hail::FileObject::operator=(FileObject& object)
{
    m_name = object.m_name;
    m_parentName = object.m_parentName;
    m_extension = object.m_extension;
    m_isDirectory = object.m_isDirectory;
    m_fileData = object.m_fileData;
    m_directoryLevel = object.m_directoryLevel;
    return *this;
}

const Hail::FileObject& Hail::FileObject::operator=(FileObject&& moveableObject) noexcept
{
    m_name = moveableObject.m_name;
    m_parentName = moveableObject.m_parentName;;
    m_extension = moveableObject.m_extension;
    m_isDirectory = moveableObject.m_isDirectory;
    m_fileData = moveableObject.m_fileData;
    m_directoryLevel = moveableObject.m_directoryLevel;
    return *this;
}

bool Hail::FileObject::operator==(const FileObject& a) const
{
    if (a.m_directoryLevel != m_directoryLevel || a.m_isDirectory != m_isDirectory)
        return false;
    if (a.m_extension != m_extension)
        return false;
    if (a.m_parentName != m_parentName)
        return false;
    if (a.m_name != m_name)
        return false;
    return true;
}

bool Hail::FileObject::operator!=(const FileObject& a) const
{
    return !(a == *this);
}

uint64_t Hail::FileObject::GetLastWriteFileTime() const
{
    uint64_t returnValue;
    returnValue = m_fileData.m_lastWriteTime.m_highDateTime;
    returnValue << 32;
    returnValue |= m_fileData.m_lastWriteTime.m_lowDateTime;
    return returnValue;
}

void Hail::FileObject::Reset()
{
    m_name = WString64();
    m_parentName = WString64();
    m_extension[0] = L'\0';
    //m_length = 0;
    m_isDirectory = false;
    m_directoryLevel = 0;
    memset(&m_fileData, 0, sizeof(CommonFileData));
}

bool Hail::FileObject::FindExtension()
{
    const uint32_t length = m_name.Length();
    if (length == 0)
    {
        return false;
    }
    m_isDirectory = true;
    int32_t seperator = length - 1;
    wchar_t currentCharacter = 0;
    while (seperator != 0)
    {
        if (currentCharacter == L'.')
        {
            m_isDirectory = false;
            break;
        }
        currentCharacter = m_name[seperator--];
    }
    if (m_isDirectory == false)
    {
        seperator++;
        m_extension = &m_name[seperator + 1];
        //m_extension[length - (seperator + 1)] = L'\0';
        memset(m_name + seperator, 0, sizeof(wchar_t) * (length - seperator));
        //m_length = seperator;
        return true;
    }
    m_extension = L"folder\0";
    return false;
}
