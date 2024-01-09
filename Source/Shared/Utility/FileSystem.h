#pragma once
#include "Containers\StaticArray\StaticArray.h"
#include "Containers\GrowingArray\GrowingArray.h"
#include "Containers\Stack\stack.h"
#include "Containers\VectorOnStack\VectorOnStack.h"
#include "String.hpp"
#include "Utility\FilePath.hpp"

namespace Hail
{
	constexpr uint32_t MAX_FILE_DEPTH = 16u;

	class FileIterator
	{
	public:
		FileIterator();
		~FileIterator();
		explicit FileIterator(const FilePath& basePath);
		//Iterates over a single folder, not entering any subdirectories
		bool IterateOverFolder();
		FilePath GetCurrentPath() const;

	protected:
		void Init();
		void DeInit();
		void InitPath(const FilePath& basePath);
		FilePath m_basePath;
		FileObject m_currentFileObject;
		uint16_t m_baseDepth = 0;
		uint16_t m_maxDepth = 0;

		bool m_osHandleIsOpen = false;
		void* m_currentFileFindData = nullptr;
	};

	class RecursiveFileIterator : public FileIterator
	{
	public:
		explicit RecursiveFileIterator(const FilePath& basePath);
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

	struct FileDirectoryWithFiles
	{
		FileObject directoryObject;
		GrowingArray<SelecteableFileObject> files;
	};

	class FileSystem
	{
	public:
		bool SetFilePath(const FilePath& basePath);
		bool SetFilePathAndInit(const FilePath& basePath, const VectorOnStack<String64, 8>& extensionsToSearchFor);
		// Gets the file directory at the selected depth (depth is relative to the base depth, so a depth of 6 and an index of 6 gives an directory at 0
		const GrowingArray<FileDirectoryWithFiles>* GetFileDirectoriesAtDepth(uint16 directoryDepth);
		// Gets the directory of the current file object on the current depth
		const GrowingArray<SelecteableFileObject>* GetCurrentFileDirectory();
		const GrowingArray<SelecteableFileObject>* GetFileDirectory(const FileObject& fileDirectory);
		const FileObject& GetCurrentFileDirectoryObject() const { return m_currentFileDirectory; }
		bool SetCurrentFileDirectory(const FileObject&);

		GrowingArray<SelecteableFileObject>& GetFilesAtCurrentDepth();
		const SelecteableFileObject& GetDirectoryAtDepth(uint16_t requestedDepth);
		const SelecteableFileObject& GetDirectoryAtCurrentDepth();
		void JumpUpOneDirectory(const SelecteableFileObject& directoryToJumpTo);
		void JumpToParent();
		void JumpToDepth(uint16_t depthToGoTo);
		void ReloadFolder(const FilePath& folderTeReload);

		//Resets all the data on the file-system
		void Reset();
		//Gets the depth the filesystem was initialized too
		uint16_t GetBaseDepth() const { return m_baseDepth; }
		uint16_t GetCurrentDepth() const { return m_currentDepth; }
		uint16_t GetMaxDepth() const { return m_maxDepth; }

		const FilePath& GetCurrentFilePath() const { return m_basePath; };

	protected:
		void Initialize();
		// This function only updates the selected path of objects, not outside of the tree structure 
		void IterateOverFolder(const FilePath& currentPath);
		// Update a selected folder of the hierarchy or create it if it does not exist, will only fill one level of data
		void IterateOverFolderAndFillData(const FilePath& pathToUpdate);
		// Creates the entire data structure for the whole system structure
		void IterateOverFilesAndFillDataRecursively(const FilePath& currentPath);
		void SetDirectories(FilePath path);

		FilePath m_basePath;
		bool m_isInitialized = false;
		// Goes from 0 and up based on the initial baseDepth
		StaticArray<GrowingArray<FileDirectoryWithFiles>, MAX_FILE_DEPTH> m_fileDirectories;
		FileObject m_currentFileDirectory;
		uint16 m_currentDepth{};
		// The files of the current selected directory, TODO: maybe remove this one, but it is fine
		GrowingArray<SelecteableFileObject> m_currentDirectoryFiles;
		// Always at 0 is the drive
		StaticArray<SelecteableFileObject, MAX_FILE_DEPTH> m_directories;
		VectorOnStack<String64, 8> m_extensionsToSearchFor;
		uint16_t m_baseDepth = 0;
		uint16_t m_maxDepth = 0;
	};

}
 