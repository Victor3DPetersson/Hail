#pragma once
#include <string>
#include "..\Containers\StaticArray\StaticArray.h"
#include "..\Containers\GrowingArray\GrowingArray.h"
#include "..\Containers\Stack\stack.h"
#include "..\Containers\VectorOnStack\VectorOnStack.h"
#include "String.hpp"
#include "Utility\FilePath.hpp"

#ifdef PLATFORM_WINDOWS
//TODO move to cpp file to rewmove windows.h from header
#include <windows.h>

#else 

#endif

namespace Hail
{
	constexpr uint32_t MAX_FILE_DEPTH = 16u;

	class FileIterator
	{
		struct FindFileData
		{
#ifdef PLATFORM_WINDOWS
			WIN32_FIND_DATA m_findFileData;
			HANDLE m_hFind;
#endif
		};
	public:
		~FileIterator();
		explicit FileIterator(FilePath basePath);
		//Iterates over a single folder, not entering any subdirectories
		bool IterateOverFolder();
		FilePath GetCurrentPath() const;

	protected:

		void InitPath(const FilePath& basePath);
		FilePath m_basePath;
		FileObject m_currentFileObject;
		uint16_t m_baseDepth = 0;
		uint16_t m_maxDepth = 0;

		FindFileData m_currentFileFindData;
		bool m_osHandleIsOpen = false;
	};

	class RecursiveFileIterator : public FileIterator
	{
	public:
		~RecursiveFileIterator();
		explicit RecursiveFileIterator(FilePath basePath);
		//Iterate over a folder and all its sub-directories. Will return true until it has iterated over all files.
		bool IterateOverFolderRecursively();


	private:
		Stack<FilePath> m_directoriesToIterateOver;
	};

	struct SelecteableFileObject
	{
		SelecteableFileObject() = default;
		SelecteableFileObject(const FileObject& fileObject);
		FileObject m_fileObject;
		bool m_selected;
	};

	class FileSystem
	{
	public:
		bool SetFilePath(FilePath basePath);
		bool SetFilePathAndInit(FilePath basePath, const VectorOnStack<String64, 8>& extensionsToSearchFor);
		bool IsInitialized() const;
		GrowingArray<SelecteableFileObject>& GetFilesAtDepth(uint16_t requestedDepth);
		const SelecteableFileObject& GetDirectoryAtDepth(uint16_t requestedDepth);
		void JumpUpOneDirectory(const SelecteableFileObject& directoryToJumpTo);
		void JumpToParent();
		void JumpToDepth(uint16_t depthToGoTo);
		uint16_t GetCurrentDepth() const { return m_baseDepth; }
		uint16_t GetMaxDepth() const { return m_maxDepth; }
		const FilePath& GetCurrentFilePath() const { return m_basePath; };
	private:
		void Initialize();
		void IterateOverFolder(FilePath currentPath);
		void IterateOverFilesRecursively(FilePath currentPath);
		void SetDirectories(FilePath path);

		FilePath m_basePath;
		bool m_isInitialized = false;
		StaticArray<GrowingArray<SelecteableFileObject>, MAX_FILE_DEPTH> m_files;
		StaticArray<SelecteableFileObject, MAX_FILE_DEPTH> m_directories;
		VectorOnStack<String64, 8> m_extensionsToSearchFor;
		uint16_t m_baseDepth = 0;
		uint16_t m_maxDepth = 0;
	};
	

}
 