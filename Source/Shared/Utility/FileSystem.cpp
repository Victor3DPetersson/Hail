#include "Shared_PCH.h"
#include "FileSystem.h"
#include "DebugMacros.h"

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


#endif


    SelecteableFileObject::SelecteableFileObject(const FileObject& fileObject) : m_fileObject(fileObject)
    {
        m_selected = false;
    }

    void FileSystem::IterateOverFilesRecursively(FilePath currentPath)
    {
        RecursiveFileIterator iterator = RecursiveFileIterator(currentPath);
        while (iterator.IterateOverFolderRecursively())
        {
            const FilePath iteratedPath = iterator.GetCurrentPath();
            const FileObject currentObject = iteratedPath.Object();
            m_files[currentPath.GetDirectoryLevel()].Add(currentObject);
        }

        //FileObject object = FileObject(currentPath);
        //if (IsValidFilePath(currentPath))
        //{
        //    WIN32_FIND_DATA FindFileData;
        //    HANDLE hFind = FindFirstFile(currentPath.Data(), &FindFileData);

        //    do {
        //        if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
        //            && wcscmp(FindFileData.cFileName, L".") != 0
        //            && wcscmp(FindFileData.cFileName, L"..") != 0)
        //        {
        //            // Call our function again to search in this sub-directory 
        //            String64 fileName;
        //            wcstombs(fileName, FindFileData.cFileName, wcslen(FindFileData.cFileName) + 1);
        //            Debug_PrintConsoleConstChar("\nDirectory:");
        //            Debug_PrintConsoleConstChar(fileName);
        //            

        //            FileObject directoryObject = FileObject(FindFileData.cFileName, currentPath.Object(), ConstructFileData(FindFileData));
        //            m_files[currentPath.GetDirectoryLevel()].Add(directoryObject);
        //            FilePath newPath = (currentPath);
        //            newPath = newPath + FindFileData.cFileName + L"*\0";
        //            IterateOverFilesRecursively(newPath);
        //        }
        //        else
        //        {
        //            if (wcscmp(FindFileData.cFileName, L".") != 0
        //                && wcscmp(FindFileData.cFileName, L"..") != 0)
        //            {
        //                FileObject fileObject = FileObject(FindFileData.cFileName, currentPath.Object(), ConstructFileData(FindFileData));
        //                m_files[currentPath.GetDirectoryLevel()].Add(fileObject);
        //                String64 file;
        //                wcstombs(file, FindFileData.cFileName, wcslen(FindFileData.cFileName) + 1);
        //                Debug_PrintConsoleConstChar(file.Data());
        //            }
        //        }
        //    } while (FindNextFile(hFind, &FindFileData) != 0);
        //    FindClose(hFind);
        //}
        //else
        //{
        //    //error message? 
        //}
    }

    void FileSystem::JumpUpOneDirectory(const SelecteableFileObject& directoryToJumpTo)
    {
        if (directoryToJumpTo.m_fileObject.IsDirectory())
        {
            WString64 name = directoryToJumpTo.m_fileObject.Name();
            if (name.Data()[name.Length() - 1] != g_SourceSeparator)
            {
                name = name + g_SourceSeparatorAndEnd;
            }
            FilePath newPath = m_basePath + name;
            if (newPath.IsValid())
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
                    if (name.Data()[name.Length() - 1] != g_SourceSeparator)
                    {
                        name = name + g_SourceSeparatorAndEnd;
                    }
                    newPath = newPath + name;
                }
            }
            if (newPath.IsValid())
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
        if (currentPath.IsValid())
        {
            m_files[currentPath.GetDirectoryLevel()].RemoveAll();
            FileIterator iterator = FileIterator(currentPath);
            while (iterator.IterateOverFolder())
            {
                const FilePath iteratedPath = iterator.GetCurrentPath();
                const FileObject currentObject = iteratedPath.Object();
                if (currentObject.IsDirectory())
                {
                    m_files[currentPath.GetDirectoryLevel()].Add(currentObject);
                }
                else
                {
                    if (m_extensionsToSearchFor.Empty())
                    {
                        m_files[currentPath.GetDirectoryLevel()].Add(currentObject);
                    }
                    else
                    {
                        bool isObjectInList = false;
                        for (uint32_t i = 0; i < m_extensionsToSearchFor.Size(); i++)
                        {
                            String64 extension;
                            WString64 wExtension = currentObject.Extension();
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
                            m_files[currentPath.GetDirectoryLevel()].Add(currentObject);
                        }
                    }
                }
            }
        }
        //currentPath.AddWildcard();
        //if (IsValidFilePath(currentPath))
        //{
        //    m_files[currentPath.GetDirectoryLevel()].RemoveAll();
        //    WIN32_FIND_DATA FindFileData;
        //    HANDLE hFind = FindFirstFile(currentPath.Data(), &FindFileData);
        //    do {
        //        if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
        //            && wcscmp(FindFileData.cFileName, L".") != 0
        //            && wcscmp(FindFileData.cFileName, L"..") != 0)
        //        {
        //            FileObject directoryObject = FileObject(FindFileData.cFileName, currentPath.Object(), ConstructFileData(FindFileData));
        //            m_files[currentPath.GetDirectoryLevel()].Add(directoryObject);
        //        }
        //        else
        //        {
        //            if (wcscmp(FindFileData.cFileName, L".") != 0
        //                && wcscmp(FindFileData.cFileName, L"..") != 0)
        //            {
        //                if (m_extensionsToSearchFor.Empty())
        //                {
        //                    FileObject fileObject = FileObject(FindFileData.cFileName, currentPath.Object(), ConstructFileData(FindFileData));
        //                    m_files[currentPath.GetDirectoryLevel()].Add(fileObject);
        //                }
        //                else
        //                {
        //                    FileObject fileObject = FileObject(FindFileData.cFileName, currentPath.Object(), ConstructFileData(FindFileData));
        //                    bool isObjectInList = false;
        //                    for (uint32_t i = 0; i < m_extensionsToSearchFor.Size(); i++)
        //                    {
        //                        String64 extension;
        //                        WString64 wExtension = fileObject.Extension();
        //                        wcstombs(extension.Data(), wExtension, wExtension.Length());
        //                        extension[wExtension.Length()] = '\0';
        //                        if (m_extensionsToSearchFor[i] == extension)
        //                        {
        //                            isObjectInList = true;
        //                            break;
        //                        }
        //                    }
        //                    if (isObjectInList)
        //                    {
        //                        m_files[currentPath.GetDirectoryLevel()].Add(fileObject);
        //                    }
        //                }

        //            }
        //        }
        //    } while (FindNextFile(hFind, &FindFileData) != 0);
        //    FindClose(hFind);
        //}
        //else
        //{
        //    //error message? 
        //}
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
        if (basePath.IsValid())
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

    FileIterator::~FileIterator()
    {
        if (m_osHandleIsOpen)
        {
            FindClose(m_currentFileFindData.m_hFind);
        }
    }

    FileIterator::FileIterator(FilePath basePath)
    {
        m_basePath = basePath;
        m_baseDepth = m_basePath.GetDirectoryLevel();
        InitPath(m_basePath);
    }

    void FileIterator::InitPath(const FilePath& basePath)
    {
        m_currentFileObject = FileObject();
        FilePath currentPath = basePath;
        currentPath.AddWildcard();

        if (currentPath.IsValid())
        {
            WIN32_FIND_DATA FindFileData;
            HANDLE hFind = FindFirstFile(currentPath.Data(), &FindFileData);
            m_osHandleIsOpen = hFind != 0;
            m_currentFileFindData.m_findFileData = FindFileData;
            m_currentFileFindData.m_hFind = hFind;
        }
        else
        {
            m_osHandleIsOpen = false;
        }
    }

    bool FileIterator::IterateOverFolder()
    {
        FilePath currentPath = m_basePath + m_currentFileObject;
        currentPath.AddWildcard();

        if (m_osHandleIsOpen)
        {
            do 
            {
                if ((m_currentFileFindData.m_findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
                    && wcscmp(m_currentFileFindData.m_findFileData.cFileName, L".") != 0
                    && wcscmp(m_currentFileFindData.m_findFileData.cFileName, L"..") != 0)
                {
                    m_currentFileObject = FileObject(m_currentFileFindData.m_findFileData.cFileName, currentPath.Object(), ConstructFileData(m_currentFileFindData.m_findFileData));
                    if (FindNextFile(m_currentFileFindData.m_hFind, &m_currentFileFindData.m_findFileData) == 0)
                    {
                        m_osHandleIsOpen = false;
                        FindClose(m_currentFileFindData.m_hFind);
                    }
                    return true;
                }
                else
                {
                    if (wcscmp(m_currentFileFindData.m_findFileData.cFileName, L".") != 0
                        && wcscmp(m_currentFileFindData.m_findFileData.cFileName, L"..") != 0)
                    {
                        m_currentFileObject = FileObject(m_currentFileFindData.m_findFileData.cFileName, currentPath.Object(), ConstructFileData(m_currentFileFindData.m_findFileData));
                        if (FindNextFile(m_currentFileFindData.m_hFind, &m_currentFileFindData.m_findFileData) == 0)
                        {
                            m_osHandleIsOpen = false;
                            FindClose(m_currentFileFindData.m_hFind);
                        }
                        return true;
                    }
                }
            } while (FindNextFile(m_currentFileFindData.m_hFind, &m_currentFileFindData.m_findFileData) != 0);
            m_osHandleIsOpen = false;
            FindClose(m_currentFileFindData.m_hFind);
            return true;
        }
        return false;
    }

    FilePath FileIterator::GetCurrentPath() const
    {
        FilePath returnPath = m_basePath;
        return returnPath + m_currentFileObject;
    }

    RecursiveFileIterator::~RecursiveFileIterator()
    {
    }

    RecursiveFileIterator::RecursiveFileIterator(FilePath basePath) : FileIterator(basePath)
    {
    }

    bool RecursiveFileIterator::IterateOverFolderRecursively()
    {
        const bool iterationResult = IterateOverFolder();
        if (iterationResult)
        {
            if (m_currentFileObject.IsDirectory())
            {
                m_directoriesToIterateOver.Push(m_basePath + m_currentFileObject);
            }
        }
        else
        {
            if (m_directoriesToIterateOver.Size() > 0)
            {
                m_basePath = m_directoriesToIterateOver.Pop();
                InitPath(m_basePath);
                Debug_PrintConsoleConstChar("\n\nNew Path");
            }
            else
            {
                return false;
            }
        }
        return true;
    }

}

