#include "Shared_PCH.h"
#include "FileSystem.h"
#include "DebugMacros.h"
#ifdef PLATFORM_WINDOWS

#include <windows.h>
//#elif PLATFORM_OSX//.... more to be added

#endif
#include <stack>
#include <string>
#include <locale> 
#include <codecvt>
#include <strsafe.h>
#include <stdarg.h>
#include "String.hpp"
#include "MathUtils.h"

namespace Hail
{
#ifdef PLATFORM_WINDOWS
    CommonFileData ConstructFileData(WIN32_FIND_DATA findData)
    {
        CommonFileData fileData;
        LARGE_INTEGER largeInteger;
        largeInteger.LowPart = findData.nFileSizeLow;
        largeInteger.HighPart = findData.nFileSizeHigh;
        fileData.m_filesizeInBytes = largeInteger.QuadPart;

        fileData.m_creationTime.m_highDateTime = findData.ftCreationTime.dwHighDateTime;
        fileData.m_creationTime.m_lowDateTime = findData.ftCreationTime.dwLowDateTime;
        fileData.m_lastWriteTime.m_highDateTime = findData.ftLastWriteTime.dwHighDateTime;
        fileData.m_lastWriteTime.m_lowDateTime = findData.ftLastWriteTime.dwLowDateTime;
        return fileData;
    }
#else 


#endif

    SelecteableFileObject::SelecteableFileObject(const FileObject& fileObject) : m_fileObject(fileObject)
    {
        m_selected = false;
    }

    void FileSystem::IterateOverFilesRecursively(FilePath currentPath)
    {
        FileObject object = FileObject(currentPath);
        if (IsValidFilePath(currentPath))
        {
            WIN32_FIND_DATA FindFileData;
            HANDLE hFind = FindFirstFile(currentPath.Data(), &FindFileData);

            do {
                if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
                    && wcscmp(FindFileData.cFileName, L".") != 0
                    && wcscmp(FindFileData.cFileName, L"..") != 0)
                {
                    // Call our function again to search in this sub-directory 
                    String64 fileName;
                    wcstombs(fileName, FindFileData.cFileName, wcslen(FindFileData.cFileName) + 1);
                    Debug_PrintConsoleConstChar("\nDirectory:");
                    Debug_PrintConsoleConstChar(fileName);
                    

                    FileObject directoryObject = FileObject(FindFileData.cFileName, currentPath.Object(), ConstructFileData(FindFileData));
                    m_files[currentPath.GetDirectoryLevel()].Add(directoryObject);
                    FilePath newPath = (currentPath);
                    newPath = newPath + FindFileData.cFileName + L"*\0";
                    IterateOverFilesRecursively(newPath);
                }
                else
                {
                    if (wcscmp(FindFileData.cFileName, L".") != 0
                        && wcscmp(FindFileData.cFileName, L"..") != 0)
                    {
                        FileObject fileObject = FileObject(FindFileData.cFileName, currentPath.Object(), ConstructFileData(FindFileData));
                        m_files[currentPath.GetDirectoryLevel()].Add(fileObject);
                        String64 file;
                        wcstombs(file, FindFileData.cFileName, wcslen(FindFileData.cFileName) + 1);
                        Debug_PrintConsoleConstChar(file.Data());
                    }
                }
            } while (FindNextFile(hFind, &FindFileData) != 0);
            FindClose(hFind);
        }
        else
        {
            //error message? 
        }
    }

    void FileSystem::JumpUpOneDirectory(const SelecteableFileObject& directoryToJumpTo)
    {
        if (directoryToJumpTo.m_fileObject.IsDirectory())
        {
            WString64 name = directoryToJumpTo.m_fileObject.Name();
            if (name.Data()[name.Length() - 1] != SourceSeparator)
            {
                name = name + SourceSeparatorAndEnd;
                //name = name + L"*\0";
            }
            FilePath newPath = m_basePath + name;
            if (IsValidFilePath(newPath))
            {
                m_basePath = newPath;
                m_baseDepth = m_basePath.GetDirectoryLevel();
                IterateOverFolder(m_basePath);
                SetDirectories(m_basePath);
            }
        }
    }

    void FileSystem::JumpToParent()
    {
        m_basePath = m_basePath.Parent();

        m_baseDepth = m_basePath.GetDirectoryLevel();
        m_directories[m_baseDepth - 1] = m_basePath.Object();
        IterateOverFolder(m_basePath);
        SetDirectories(m_basePath);
    }

    void FileSystem::JumpToDepth(uint16_t depthToGoTo)
    {
        if (depthToGoTo <= m_maxDepth && depthToGoTo != m_basePath.GetDirectoryLevel())
        {
            FilePath newPath = m_basePath;
            if (depthToGoTo < m_basePath.GetDirectoryLevel())
            {
                while (newPath.GetDirectoryLevel() != depthToGoTo)
                {
                    newPath = newPath.Parent();
                }
            }
            else
            {
                for (uint16_t i = m_basePath.GetDirectoryLevel(); i < depthToGoTo; i++)
                {
                    WString64 name = m_directories[i].m_fileObject.Name();
                    if (name.Data()[name.Length() - 1] != SourceSeparator)
                    {
                        name = name + SourceSeparatorAndEnd;
                    }
                    newPath = newPath + name;
                }
            }
            if (IsValidFilePath(newPath))
            {
                m_basePath = newPath;
                m_baseDepth = m_basePath.GetDirectoryLevel();
                IterateOverFolder(m_basePath);
                SetDirectories(m_basePath);
            }
        }
    }

    void FileSystem::IterateOverFolder(FilePath currentPath)
    {
        currentPath.AddWildcard();

        if (IsValidFilePath(currentPath))
        {
            m_files[currentPath.GetDirectoryLevel()].RemoveAll();
            WIN32_FIND_DATA FindFileData;
            HANDLE hFind = FindFirstFile(currentPath.Data(), &FindFileData);
            do {
                if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
                    && wcscmp(FindFileData.cFileName, L".") != 0
                    && wcscmp(FindFileData.cFileName, L"..") != 0)
                {
                    FileObject directoryObject = FileObject(FindFileData.cFileName, currentPath.Object(), ConstructFileData(FindFileData));
                    m_files[currentPath.GetDirectoryLevel()].Add(directoryObject);
                }
                else
                {
                    if (wcscmp(FindFileData.cFileName, L".") != 0
                        && wcscmp(FindFileData.cFileName, L"..") != 0)
                    {
                        if (m_extensionsToSearchFor.Empty())
                        {
                            FileObject fileObject = FileObject(FindFileData.cFileName, currentPath.Object(), ConstructFileData(FindFileData));
                            m_files[currentPath.GetDirectoryLevel()].Add(fileObject);
                        }
                        else
                        {
                            FileObject fileObject = FileObject(FindFileData.cFileName, currentPath.Object(), ConstructFileData(FindFileData));
                            bool isObjectInList = false;
                            for (uint32_t i = 0; i < m_extensionsToSearchFor.Size(); i++)
                            {
                                String64 extension;
                                WString64 wExtension = fileObject.Extension();
                                wcstombs(extension.Data(), wExtension, wExtension.Length());
                                extension[wExtension.Length()] = '\0';
                                if (m_extensionsToSearchFor[i] == extension)
                                {
                                    isObjectInList = true;
                                    break;
                                }
                            }
                            if (isObjectInList)
                            {
                                m_files[currentPath.GetDirectoryLevel()].Add(fileObject);
                            }
                        }

                    }
                }
            } while (FindNextFile(hFind, &FindFileData) != 0);
            FindClose(hFind);
        }
        else
        {
            //error message? 
        }
    }

    void FileSystem::SetDirectories(FilePath path)
    {
        m_maxDepth = Math::Max(m_maxDepth, path.GetDirectoryLevel());
        while (path.GetDirectoryLevel() != 0)
        {
            m_directories[path.GetDirectoryLevel() - 1] = path.Object();
            if (path.GetDirectoryLevel() == 1)
            {
                break;
            }
            path = path.Parent();
        }
    }

    void FileSystem::Initialize()
    {
        if (!m_isInitialized)
        {
            for (uint32_t i = 0; i < MAX_FILE_DEPTH; i++)
            {
                m_files[i].Init(32);
            }
            m_isInitialized = true;
        }
    }

    bool FileSystem::SetFilePath(FilePath basePath)
    {
        if (basePath.IsValid() && IsValidFilePath(basePath))
        {
            Initialize();
            m_basePath = basePath;
            SetDirectories(m_basePath);
            m_baseDepth = m_basePath.GetDirectoryLevel();
            IterateOverFolder(m_basePath);
            return true;
        }
        return false;
    }

    bool FileSystem::SetFilePathAndInit(FilePath basePath, const VectorOnStack<String64, 8>& extensionsToSearchFor)
    {
        m_extensionsToSearchFor.Clear();
        if (SetFilePath(basePath))
        {
            m_extensionsToSearchFor = extensionsToSearchFor;
            return true;
        }
        return false;
    }


    bool FileSystem::IsInitialized() const
    {
        return m_isInitialized;
    }

    GrowingArray<SelecteableFileObject>& FileSystem::GetFilesAtDepth(uint16_t requestedDepth)
    {
        return m_files[requestedDepth];
    }

    const SelecteableFileObject& FileSystem::GetDirectoryAtDepth(uint16_t requestedDepth)
    {
        return m_directories[requestedDepth];
    }

    bool FileSystem::IsValidFilePath(FilePath pathToCheck) const
    {
        pathToCheck.AddWildcard();
        //IfDef Windows here
        WIN32_FIND_DATA FindFileData;
        HANDLE hFind = FindFirstFile(pathToCheck.Data(), &FindFileData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            Debug_PrintConsoleConstChar("File not valid\n");
            return false;
        }
        return true;
    }


    //bool ListFiles(FilePath path, std::wstring mask/*, vector<wstring>& files*/)
    //{

    //    WIN32_FIND_DATA FindFileData;
    //    HANDLE hFind = FindFirstFile(path.Data(), &FindFileData);
    //    if (hFind == INVALID_HANDLE_VALUE) {

    //        Debug_PrintConsoleConstChar("File not valid\n");
    //        return false;
    //    }
    //    IterateOverFilesRecursively(path);
    //    FindClose(hFind);




    //    return true;
    //}

}

