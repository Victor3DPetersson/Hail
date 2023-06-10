#include "Shared_PCH.h"
#include "FilePath.hpp"

#include <string>
#include <iostream>

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
        while (seperator != 0)
        {
            if (isDirectory && !foundFirstSeperator)
            {
                if (currentCharacter == Hail::SourceSeparator)
                {
                    currentCharacter = L'\0';
                    foundFirstSeperator = true;
                }
            }
            if (currentCharacter == Hail::SourceSeparator)
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
    if (m_isDirectory && m_data[m_length - 1] != Wildcard)
    {
        m_data[m_length++] = Wildcard;
        m_data[m_length] = L'\0';
    }
}

Hail::FilePath::FilePath(const FilePath& path, uint32_t lengthOfPath)
{
    *this = path;
    m_directoryLevel--;
    for (; ; --m_length)
    {
        m_data[m_length] = L'\0';
        lengthOfPath--;
        if (lengthOfPath == 0)
        {
            break;
        }
    }
}

Hail::FilePath Hail::FilePath::operator+(const FilePath& otherPath)
{
    if (otherPath.IsValid())
    {
        if (m_isDirectory)
        {
            if (m_data[m_length - 1] == Wildcard)
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

Hail::FilePath Hail::FilePath::operator+(const FileObject& object)
{
    uint32_t length = 0;
    if (object.IsValid() && m_isDirectory)
    {
        if (m_data[m_length - 1] == Wildcard)
        {
            m_data[--m_length] = L'\0';
        }
        length += object.Name().Length();
        wcscat_s(m_data, MAX_FILE_LENGTH - m_length, object.Name());
        if (object.IsDirectory())
        {
            m_data[m_length + length++] = SourceSeparator;
        }
        else
        {
            m_data[m_length + length++] = L'.';
            m_data[m_length + length] = End;
            length += object.Extension().Length();
            wcscat_s(m_data, MAX_FILE_LENGTH - (m_length + length), object.Extension());
        }
        m_data[m_length + length] = End;
        m_length += length;
        FindExtension();
    }
    return *this;
}

Hail::FilePath Hail::FilePath::operator+(const wchar_t* const string)
{
    uint32_t length = wcslen(string);
    if (length > 0)
    {
        if (m_isDirectory)
        {
            if (m_data[m_length - 1] == Wildcard)
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
    if (IsValid())
    {
        if (m_directoryLevel > 1)
        {
            const int objectLength = GetParentSeperator(m_data, m_length, m_isDirectory);
            FilePath path = FilePath(*this, objectLength);
            path.CreateFileObject();
            return path;
        }
        else
        {
            //No parent directory
            return *this;
        }
    }
    return FilePath();
}

void Hail::FilePath::Slashify()
{
    size_t size = wcslen(m_data);
    size_t i(0);
    if (size == 0)
    {
        return;
    }
    m_directoryLevel = 0;
    wchar_t temp[MAX_FILE_LENGTH];
    temp[0] = L'\0';
    wcscpy_s(temp, m_data);
    uint32_t counter = 0;
    for (; i < size; i++)
    {
        if ((temp[i] == SourceSeparator || temp[i] == Separator) && temp[i + 1] != SourceSeparator && temp[i + 1] != Separator)
        {
            m_data[counter++] = SourceSeparator;
            m_directoryLevel++;
        }
        else if (m_data[i] != SourceSeparator && m_data[i] != Separator)
        {
            m_data[counter++] = temp[i];
        }
    }
    for (i = 0; i < counter; i++)
    {
        if (m_data[i] == Separator && m_data[i] != SourceSeparator )
        {
            m_data[i] = SourceSeparator;
        }
    }
    if (m_isDirectory)
    {
        if (m_data[counter - 1] != SourceSeparator && m_data[counter - 1] != L'*')
        {
            m_data[counter] = SourceSeparator;
            m_data[++counter] = L'\0';
        }
    }
    m_length = counter;
    //m_directoryLevel--;
    //wcscat_s(m_data, MAX_FILE_LENGTH - counter, L"\\*\0");
}

//const wchar_t* Hail::FilePath::GetObject() const
//{
//    if (IsValid())
//    {
//        const int objectLength = GetParentSeperator(m_data, m_length, m_isDirectory);
//        return m_data + (m_length - objectLength);
//    }
//    return nullptr;
//}

bool Hail::FilePath::IsValid() const
{
    return m_length != 0;
}

bool Hail::FilePath::FindExtension()
{
    if (m_length == 0)
    {
        return false;
    }
    m_isDirectory = true;
    int32_t seperator = m_length - 1;
    wchar_t currentCharacter = 0;
    while (seperator != 0)
    {
        if (currentCharacter == Wildcard)
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
    if (m_isDirectory == false)
    {
        return true;
    }
    return false;
}

bool Hail::FileObject::IsValid() const
{
    return m_name.Length() != 0;
}

void Hail::FilePath::Reset()
{
    m_data[0] = L'\0';
    m_length = 0;
    m_isDirectory = false;
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
    if(!filePath.IsValid())
    {
        return;
    }

    if (filePath.GetDirectoryLevel() <= 1)
    {
        m_name.Data()[0] = filePath.Data()[0];
        m_name.Data()[1] = L'\0';
        m_directoryLevel = 1;
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
        if (filePath.Data()[filePath.Length() - 1] == L'*')
        {
            m_name.Data()[length - 2] = L'\0';
            //m_length--;
        }
        m_name.Data()[length - 1] = L'\0';
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
            if (currentCharacter == L'*')
            {
                break;
            }
            if (currentCharacter == L'.')
            {
                m_isDirectory = false;
                break;
            }
            currentCharacter = m_name[seperator--];
        }
        const uint32_t fileNameLength = m_name.Length() - (seperator + 1);
        memcpy(m_extension, m_name + fileNameLength + 1, sizeof(wchar_t) * seperator);
        m_extension[seperator] = L'\0';
        memset(m_name + fileNameLength, 0, sizeof(wchar_t) * (seperator + 1));
    }

    if (filePath.GetDirectoryLevel() == 2)
    {
        int32_t parentParentSeperator = 0;
        memcpy(m_parentName, filePath.Data() + ((filePath.Length() - parentSeperator)) + 1, sizeof(wchar_t) * (parentSeperator));
        m_parentName[parentSeperator - 2] = L'\0';
    }
    else
    {
        int32_t parentParentSeperator = GetParentSeperator(filePath.Data(), (filePath.Length() - parentSeperator) + 1, true);
        memcpy(m_parentName, filePath.Data() + ((filePath.Length() - parentSeperator) - parentParentSeperator) + 2, sizeof(wchar_t) * (parentParentSeperator));
        m_parentName[parentParentSeperator - 2] = L'\0';
    }
    m_directoryLevel = filePath.GetDirectoryLevel();
}

Hail::FileObject::FileObject(const wchar_t* const string, const FileObject& parentObject, CommonFileData fileData) :
    m_fileData(fileData)
{
    Reset();
    m_fileData = fileData;
    m_name = string;
    m_parentName = parentObject.Name();

    FindExtension();
}

Hail::FileObject& Hail::FileObject::operator=(const FileObject& object)
{
    m_name = object.m_name;
    m_parentName = object.m_parentName;
    m_extension = object.m_extension;
    //m_length = object.m_length;
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
