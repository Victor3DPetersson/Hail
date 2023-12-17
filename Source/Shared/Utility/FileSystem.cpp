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

#ifdef PLATFORM_WINDOWS
#include <windows.h>

#else 

#endif

namespace Hail
{
    struct FindFileData
    {
#ifdef PLATFORM_WINDOWS
        WIN32_FIND_DATA m_findFileData;
#endif
        void* m_hFind;
    };

    SelecteableFileObject::SelecteableFileObject(const FileObject& fileObject) : m_fileObject(fileObject)
    {
        m_selected = false;
    }

    void FileSystem::IterateOverFilesAndFillDataRecursively(const FilePath& currentPath)
    {
        FileDirectoryWithFiles currentDirectory;
        currentDirectory.files.Init(8);
        RecursiveFileIterator iterator = RecursiveFileIterator(currentPath);
        {
            const FilePath iteratedPath = iterator.GetCurrentPath();
            const FileObject baseObject = iteratedPath.Object();
            const uint16 baseDepth = baseObject.GetDirectoryLevel() - m_baseDepth;
            currentDirectory.directory = baseObject;
            // assert if baseDepth != 0

            m_fileDirectories[baseDepth].Init(1);
            m_fileDirectories[baseDepth].Add(currentDirectory);

        }
        uint16 currentDirectoryDepth = 0;
        while (iterator.IterateOverFolderRecursively())
        {

            const FilePath iteratedPath = iterator.GetCurrentPath();
            const FileObject currentObject = iteratedPath.Object();

            const uint16 currentDepth = iteratedPath.GetDirectoryLevel() - m_baseDepth;

            if (currentObject.GetDirectoryLevel() - m_baseDepth != currentDirectoryDepth + 1)
            {
                currentDirectoryDepth++;
                m_maxDepth = Math::Max(m_maxDepth, currentObject.GetDirectoryLevel());
            }

            if (currentObject.IsDirectory())
            {
                if (currentObject != currentDirectory.directory)
                {
                    currentDirectory.files.RemoveAll();
                    currentDirectory.directory = currentObject;
                }
                if (!m_fileDirectories[currentDepth].IsInitialized())
                {
                    m_fileDirectories[currentDepth].Init(4);
                }
                m_fileDirectories[currentDepth].Add(currentDirectory);
            }

            for (size_t i = 0; i < m_fileDirectories[currentDirectoryDepth].Size(); i++)
            {
                if (m_fileDirectories[currentDirectoryDepth][i].directory.Name() == currentObject.ParentName())
                {
                    m_fileDirectories[currentDirectoryDepth][i].files.Add(currentObject);
                    break;
                }
            }
        }
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
                m_currentDepth = m_basePath.GetDirectoryLevel();
                m_currentFileDirectory = directoryToJumpTo.m_fileObject;
                IterateOverFolder(m_basePath);
                SetDirectories(m_basePath);
            }
        }
    }

    void FileSystem::JumpToParent()
    {
        m_basePath = m_basePath.Parent();

        m_currentDepth = m_basePath.GetDirectoryLevel();
        m_directories[m_currentDepth - 1] = m_basePath.Object();
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
                    WString64 name = m_directories[i + 1].m_fileObject.Name();
                    if (name.Data()[name.Length() - 1] != g_SourceSeparator)
                    {
                        name = name + g_SourceSeparatorAndEnd;
                    }
                    newPath = newPath + m_directories[i + 1].m_fileObject.Name();
                }
            }
            if (newPath.IsValid())
            {
                m_basePath = newPath;
                m_currentFileDirectory = m_basePath.Object();
                m_currentDepth = m_basePath.GetDirectoryLevel();
                IterateOverFolder(m_basePath);
                SetDirectories(m_basePath);
            }
        }
    }
    const GrowingArray<FileDirectoryWithFiles>* FileSystem::GetFileDirectoriesAtDepth(uint16 directoryDepth)
    {
        if (!m_isInitialized || directoryDepth < m_baseDepth || directoryDepth > m_maxDepth)
            return nullptr;
        return &m_fileDirectories[directoryDepth - m_baseDepth];
    }

    const GrowingArray<SelecteableFileObject>* FileSystem::GetCurrentFileDirectory()
    {
        if (!m_isInitialized || m_currentDepth < m_baseDepth)
            return nullptr;

        for (size_t i = 0; i < m_fileDirectories[m_currentDepth - m_baseDepth].Size(); i++)
        {
            if (m_fileDirectories[m_currentDepth - m_baseDepth][i].directory.Name() == m_currentFileDirectory.Name())
            {
                return &m_fileDirectories[m_currentDepth - m_baseDepth][i].files;
            }
        }
    }

    const GrowingArray<SelecteableFileObject>* FileSystem::GetFileDirectory(const FileObject& fileDirectory)
    {
        if (!m_isInitialized)
            return nullptr;
        if (fileDirectory.GetDirectoryLevel() < m_baseDepth)
            return nullptr;
        for (size_t i = 0; i < m_fileDirectories[fileDirectory.GetDirectoryLevel() - m_baseDepth].Size(); i++)
        {
            if (m_fileDirectories[fileDirectory.GetDirectoryLevel() - m_baseDepth][i].directory.Name() == fileDirectory.Name())
            {
                return &m_fileDirectories[fileDirectory.GetDirectoryLevel() - m_baseDepth][i].files;
            }
        }
    }

    bool FileSystem::SetCurrentFileDirectory(const FileObject& directory)
    {
        int16 directoryLevel;
        if (directory.IsDirectory())
        {
            directoryLevel = directory.GetDirectoryLevel() - m_baseDepth;
        }
        else
        {
            if (directory.GetDirectoryLevel() == 0)
                return false;
            directoryLevel = (directory.GetDirectoryLevel() - m_baseDepth) - 1;
        }
        bool foundDirectory = false;
        if (directoryLevel >= 0)
        {
            for (size_t i = 0; i < m_fileDirectories[directoryLevel].Size(); i++)
            {
                if (m_fileDirectories[directoryLevel][i].directory.Name() == directory.Name())
                {
                    m_currentFileDirectory = directory;
                    m_currentDepth = directoryLevel + m_baseDepth;
                    foundDirectory = true;
                    break;
                }
            }
        }
        else
        {
            if (m_directories[directory.GetDirectoryLevel()].m_fileObject.Name() == directory.Name())
            {
                m_currentFileDirectory = directory;
                m_currentDepth = directoryLevel + m_baseDepth;
                foundDirectory = true;
            }
        }

        if (foundDirectory)
        {
            if (m_basePath.Object() != directory)
            {
                m_basePath = FilePath();
                for (size_t i = 0; i < directory.GetDirectoryLevel(); i++)
                {
                    m_basePath = m_basePath + m_directories[i].m_fileObject;
                }
                m_basePath = m_basePath + directory;
            }
            IterateOverFolder(m_basePath);
            SetDirectories(m_basePath);
        }

        return foundDirectory;
    }

    void FileSystem::IterateOverFolder(const FilePath& currentPath)
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
    }

    void FileSystem::SetDirectories(FilePath path)
    {
        m_maxDepth = Math::Max(m_maxDepth, path.GetDirectoryLevel());
        while (path.GetDirectoryLevel() != 0)
        {
            m_directories[path.GetDirectoryLevel()] = path.Object();
            if (path.GetDirectoryLevel() == 1)
            {
                break;
            }
            path = path.Parent();
        }
        m_directories[0] = path.Parent().Object();
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

    bool FileSystem::SetFilePath(const FilePath& basePath)
    {
        m_extensionsToSearchFor.Clear();
        m_baseDepth = basePath.GetDirectoryLevel();
        if (basePath.IsValid() && basePath.IsDirectory())
        {
            Initialize();
            m_basePath = basePath;
            SetDirectories(m_basePath);
            m_currentDepth = m_basePath.GetDirectoryLevel();
            IterateOverFolder(m_basePath);

            IterateOverFilesAndFillDataRecursively(m_basePath);
            if (!SetCurrentFileDirectory(m_basePath.Object()))
            {
                // TODO make a cleanup function
                return false;
            }
            return true;
        }
        m_baseDepth = 0;
        m_isInitialized = false;
        return false;
    }

    bool FileSystem::SetFilePathAndInit(const FilePath& basePath, const VectorOnStack<String64, 8>& extensionsToSearchFor)
    {
        if (SetFilePath(basePath))
        {
            m_extensionsToSearchFor = extensionsToSearchFor;
            return true;
        }
        return false;
    }


    GrowingArray<SelecteableFileObject>& FileSystem::GetFilesAtDepth(uint16_t requestedDepth)
    {
        return m_files[requestedDepth];
    }

    GrowingArray<SelecteableFileObject>& FileSystem::GetFilesAtCurrentDepth()
    {
        return GetFilesAtDepth(m_currentDepth);
    }

    const SelecteableFileObject& FileSystem::GetDirectoryAtDepth(uint16_t requestedDepth)
    {
        return m_directories[requestedDepth];
    }

    const SelecteableFileObject& FileSystem::GetDirectoryAtCurrentDepth()
    {
        return GetDirectoryAtDepth(m_currentDepth);
    }

    FileIterator::FileIterator()
    {
        Init();
    }

    FileIterator::~FileIterator()
    {
        DeInit();
    }

    FileIterator::FileIterator(const FilePath& basePath)
    {
        Init();
        m_basePath = basePath;
        m_baseDepth = m_basePath.GetDirectoryLevel();
        InitPath(m_basePath);
        if (m_osHandleIsOpen)
            m_currentFileObject = basePath.Object();
    }

    void FileIterator::Init()
    {
        //TODO: make sure that this object is in pooled memory to prevent fragmentation
        m_currentFileFindData = new FindFileData();
    }

    void FileIterator::DeInit()
    {
        if (m_osHandleIsOpen)
        {
            FindClose(((FindFileData*)m_currentFileFindData)->m_hFind);
        }
        delete m_currentFileFindData;
        m_currentFileFindData = nullptr;
    }

    void FileIterator::InitPath(const FilePath& basePath)
    {
        m_currentFileObject = FileObject();
        FilePath currentPath = basePath;
        currentPath.AddWildcard();

        if (currentPath.IsValid())
        {
            WIN32_FIND_DATA findFileData;
            HANDLE hFind = FindFirstFile(currentPath.Data(), &findFileData);
            m_osHandleIsOpen = hFind != 0;
            ((FindFileData*)m_currentFileFindData)->m_findFileData = findFileData;
            ((FindFileData*)m_currentFileFindData)->m_hFind = hFind;
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

        FindFileData& currentFileData = *((FindFileData*)m_currentFileFindData);
        if (m_osHandleIsOpen)
        {
            do 
            {
                if ((currentFileData.m_findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
                    && wcscmp(currentFileData.m_findFileData.cFileName, L".") != 0
                    && wcscmp(currentFileData.m_findFileData.cFileName, L"..") != 0)
                {
                    m_currentFileObject = FileObject(currentFileData.m_findFileData.cFileName, m_basePath.Object(), ConstructFileData(&currentFileData.m_findFileData));
                    if (FindNextFile(currentFileData.m_hFind, &currentFileData.m_findFileData) == 0)
                    {
                        m_osHandleIsOpen = false;
                        FindClose(currentFileData.m_hFind);
                    }
                    return true;
                }
                else
                {
                    if (wcscmp(currentFileData.m_findFileData.cFileName, L".") != 0
                        && wcscmp(currentFileData.m_findFileData.cFileName, L"..") != 0)
                    {
                        m_currentFileObject = FileObject(currentFileData.m_findFileData.cFileName, m_basePath.Object(), ConstructFileData(&currentFileData.m_findFileData));
                        if (FindNextFile(currentFileData.m_hFind, &currentFileData.m_findFileData) == 0)
                        {
                            m_osHandleIsOpen = false;
                            FindClose(currentFileData.m_hFind);
                        }
                        return true;
                    }
                }
            } while (FindNextFile(currentFileData.m_hFind, &currentFileData.m_findFileData) != 0);
            m_osHandleIsOpen = false;
            FindClose(currentFileData.m_hFind);
            return true;
        }
        return false;
    }

    FilePath FileIterator::GetCurrentPath() const
    {
        FilePath returnPath = m_basePath;
        return returnPath + m_currentFileObject;
    }

    RecursiveFileIterator::RecursiveFileIterator(const FilePath& basePath) : FileIterator(basePath)
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
                IterateOverFolderRecursively();
            }
            else
            {
                return false;
            }
        }
        return true;
    }



}

