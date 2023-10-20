#include "Shared_PCH.h"
#include "FileData.h"
#include "FilePath.hpp"

#ifdef PLATFORM_WINDOWS

#include <windows.h>
//#elif PLATFORM_OSX//.... more to be added

#endif


bool Hail::IsValidFilePathInternal(const FilePath* pathToCheck)
{
    //TODO: Make sure that this function does not create a copy as that feels wasteful
    FilePath pathWithWildcard = *pathToCheck;
    pathWithWildcard.AddWildcard();
    //IfDef Windows here
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(pathWithWildcard.Data(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        return false;
    }
    FindClose(hFind);
    return true;
}

Hail::CommonFileData Hail::ConstructFileData(void* findData)
{

#ifdef PLATFORM_WINDOWS

    WIN32_FIND_DATA windowsFindData = *(WIN32_FIND_DATA*)findData;
    CommonFileData fileData;
    LARGE_INTEGER largeInteger;
    largeInteger.LowPart = windowsFindData.nFileSizeLow;
    largeInteger.HighPart = windowsFindData.nFileSizeHigh;
    fileData.m_filesizeInBytes = largeInteger.QuadPart;

    fileData.m_creationTime.m_highDateTime = windowsFindData.ftCreationTime.dwHighDateTime;
    fileData.m_creationTime.m_lowDateTime = windowsFindData.ftCreationTime.dwLowDateTime;
    fileData.m_lastWriteTime.m_highDateTime = windowsFindData.ftLastWriteTime.dwHighDateTime;
    fileData.m_lastWriteTime.m_lowDateTime = windowsFindData.ftLastWriteTime.dwLowDateTime;
    return fileData;

#endif
}

Hail::CommonFileData Hail::ConstructFileDataFromPath(const FilePath& path)
{
    if (path.IsValid())
    {
        
        WIN32_FIND_DATA FindFileData;
        HANDLE hFind = FindFirstFile(path.Data(), &FindFileData);
        if (hFind != 0)
        {
            CommonFileData fileData;
            LARGE_INTEGER largeInteger;
            largeInteger.LowPart = FindFileData.nFileSizeLow;
            largeInteger.HighPart = FindFileData.nFileSizeHigh;
            fileData.m_filesizeInBytes = largeInteger.QuadPart;

            fileData.m_creationTime.m_highDateTime = FindFileData.ftCreationTime.dwHighDateTime;
            fileData.m_creationTime.m_lowDateTime = FindFileData.ftCreationTime.dwLowDateTime;
            fileData.m_lastWriteTime.m_highDateTime = FindFileData.ftLastWriteTime.dwHighDateTime;
            fileData.m_lastWriteTime.m_lowDateTime = FindFileData.ftLastWriteTime.dwLowDateTime;
            return fileData;
        }

    }
    return CommonFileData();
}
