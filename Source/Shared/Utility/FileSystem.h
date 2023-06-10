#pragma once
#include <string>
#include "..\Containers\GrowingArray\GrowingArray.h"
#include "..\Containers\StaticArray\StaticArray.h"
#include "..\Containers\VectorOnStack\VectorOnStack.h"
#include "String.hpp"
#include "Utility\FilePath.hpp"
namespace Hail
{
	constexpr uint32_t MAX_FILE_DEPTH = 16u;

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
		//void Iterate();
		bool IsInitialized() const;
		GrowingArray<SelecteableFileObject>& GetFilesAtDepth(uint16_t requestedDepth);
		const SelecteableFileObject& GetDirectoryAtDepth(uint16_t requestedDepth);
		void JumpUpOneDirectory(const SelecteableFileObject& directoryToJumpTo);
		void JumpToParent();
		void JumpToDepth(uint16_t depthToGoTo);
		uint16_t GetCurrentDepth() const { return m_baseDepth; }
		uint16_t GetMaxDepth() const { return m_maxDepth; }
		const FilePath& GetCurrentFilePath() const { return m_basePath; };
		bool IsValidFilePath(FilePath pathToCheck) const;
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
	//bool ListFiles(FilePath path, std::wstring mask/*, vector<wstring>& files*/);
}
 