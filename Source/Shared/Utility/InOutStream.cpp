#include "Shared_PCH.h"
#include "InOutStream.h"

#include <stdio.h>
#include <stdlib.h>
#include "StringUtility.h"

#include <sys/types.h>
#include <sys/stat.h>



namespace
{
    const char* GetReadFlagFromEnum(Hail::FILE_OPEN_TYPE openType, bool binaryMode)
    {
        switch (openType)
        {
        case Hail::FILE_OPEN_TYPE::READ:
            return binaryMode ? "rb" : "r";
        case Hail::FILE_OPEN_TYPE::WRITE:
            return binaryMode ? "wb" : "w";
        case Hail::FILE_OPEN_TYPE::APPENDS:
            return binaryMode ? "ab" : "a";
        case Hail::FILE_OPEN_TYPE::READ_WRITE:
            return binaryMode ? "r+b" : "r+";
        case Hail::FILE_OPEN_TYPE::CLEAR_READ_WRITE:
            return binaryMode ? "w+b" : "w+";
        case Hail::FILE_OPEN_TYPE::READ_APPEND:
            return binaryMode ? "a+b" : "a+";
        }
    }
    FILE* Open(const char* szPath, const char* type)
    {
        return fopen(szPath, type);
    }
    int Close(FILE* f)
    {
        return fclose(f);
    }
}

Hail::InOutStream::~InOutStream()
{
    CloseFile();
}

bool Hail::InOutStream::OpenFile(FilePath fileToWriteTo, FILE_OPEN_TYPE wayToOpenFile, bool binaryMode)
{
    const bool alreadyOpenedFile = m_fileAction != FILE_OPEN_TYPE::NONE;
    if (alreadyOpenedFile)
    {
        return false;
    }
    if (!fileToWriteTo.CreateFileDirectory())
    {
        return false;
    }
    const bool canCreateFile = wayToOpenFile == FILE_OPEN_TYPE::APPENDS || wayToOpenFile == FILE_OPEN_TYPE::CLEAR_READ_WRITE || wayToOpenFile == FILE_OPEN_TYPE::WRITE;
    if (canCreateFile)
    {
        if (fileToWriteTo.IsDirectory())
        {
            return false;
        }
    }
    else
    {
        if (fileToWriteTo.IsDirectory() || !fileToWriteTo.IsValid())
        {
            return false;
        }
    }
    m_isBinary = binaryMode;
    char fileDirectory[MAX_FILE_LENGTH];
    FromWCharToConstChar(fileToWriteTo.Data(), fileDirectory, MAX_FILE_LENGTH);
    FILE* openedFile = Open(fileDirectory, GetReadFlagFromEnum(wayToOpenFile, binaryMode));

    if (openedFile == nullptr)
    {
        return false;
    }
    m_fileAction = wayToOpenFile;
    m_fileSize = 0;
    if (IsReading())
    {
        if (binaryMode)
        {
            struct stat stbuf;
            int fd;
            fd = fileno(openedFile); //if you have a stream (e.g. from fopen), not a file descriptor.
            if ((fstat(fd, &stbuf) != 0)) {
                /* Handle error */
            }
            m_fileSize = stbuf.st_size;
        }
        else
        {
            while (true)
            {
                fgetc(openedFile);
                if (feof(openedFile)) {
                    break;
                }
                m_fileSize++;
            }
            fseek(openedFile, 0, SEEK_SET);
        }

        //fseek(openedFile, 0L, SEEK_END);
        //m_fileSize = ftell(openedFile);
        //fseek(openedFile, 0, SEEK_SET);

    }
    m_currentPosition = 0;
    m_fileHandle = openedFile;
    return true;
}

void Hail::InOutStream::CloseFile()
{
    if (m_fileAction != FILE_OPEN_TYPE::NONE)
    {
        Close((FILE*)m_fileHandle);
        m_fileHandle = nullptr;
    }
    m_fileSize = 0;
    m_currentPosition = 0;
    m_fileAction = FILE_OPEN_TYPE::NONE;
}

bool Hail::InOutStream::Read(void* readOutData, size_t sizeOfData, size_t numberOfElements)
{
    if (!IsReading())
    {
        return false;
    }
    if (m_currentPosition + sizeOfData * numberOfElements > m_fileSize)
    {
        return false;
    }
    m_currentPosition += sizeOfData * numberOfElements;
    fread(readOutData, sizeOfData, numberOfElements, (FILE*)m_fileHandle);
    return true;
}

bool Hail::InOutStream::Write(const void* writeOutData, size_t sizeOfData, size_t numberOfElements)
{
    if (!IsWriting())
    {
        return false;
    }
    m_currentPosition += sizeOfData * numberOfElements;
    fwrite(writeOutData, sizeOfData, numberOfElements, (FILE*)m_fileHandle);
    return true;
}

bool Hail::InOutStream::Seek(int64 sizeOfData, int64 numberOfElements)
{
    if (IsReading() && m_currentPosition + sizeOfData * numberOfElements < m_fileSize)
    {
        int result = fseek((FILE*)m_fileHandle, sizeOfData * numberOfElements, SEEK_CUR);
        if (result == 0)
        {
            m_currentPosition += sizeOfData * numberOfElements;
            return true;
        }
    }
    return false;
}

void Hail::InOutStream::SeekToStart()
{
    if (m_fileAction != FILE_OPEN_TYPE::NONE)
    {
        m_currentPosition = 0;
        fseek((FILE*)m_fileHandle, 0, SEEK_SET);
    }
}

void Hail::InOutStream::SeekToEnd()
{
    if (m_fileAction != FILE_OPEN_TYPE::NONE)
    {
        m_currentPosition = 0;
        fseek((FILE*)m_fileHandle, 0, SEEK_END);
        m_currentPosition = m_fileSize;
    }
}

bool Hail::InOutStream::IsReading()
{
    return m_fileAction != FILE_OPEN_TYPE::NONE && m_fileAction == FILE_OPEN_TYPE::READ || m_fileAction == FILE_OPEN_TYPE::READ_WRITE || m_fileAction == FILE_OPEN_TYPE::CLEAR_READ_WRITE || m_fileAction == FILE_OPEN_TYPE::READ_APPEND;
}

bool Hail::InOutStream::IsWriting()
{
    return m_fileAction != FILE_OPEN_TYPE::NONE && m_fileAction == FILE_OPEN_TYPE::WRITE || m_fileAction == FILE_OPEN_TYPE::APPENDS || m_fileAction == FILE_OPEN_TYPE::READ_WRITE || m_fileAction == FILE_OPEN_TYPE::CLEAR_READ_WRITE;
}