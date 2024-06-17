#pragma once
#include <ios>
#include "Types.h"
#include "FilePath.hpp"
namespace Hail
{
	enum class FILE_OPEN_TYPE
	{
		READ,// Opens a file for reading.The file must exist.
		WRITE, //Creates an empty file for writing.If a file with the same name already exists, its content is erased and the file is considered as a new empty file.
		APPENDS, //Appends to a file. Writing operations, append data at the end of the file.The file is created if it does not exist.
		READ_WRITE, //Opens a file to update both reading and writing.The file must exist.
		CLEAR_READ_WRITE, //Creates an empty file for both reading and writing.
		READ_APPEND, //Opens a file for reading and appending.
		NONE
	};
	class InOutStream
	{
	public:
		~InOutStream();
		bool OpenFile(FilePath fileToWriteTo, FILE_OPEN_TYPE wayToOpenFile, bool binaryMode);
		void CloseFile();
		size_t GetFileSize() const { return m_fileSize; }
		size_t GetFileSeekPosition() const { return m_currentPosition; }

		//This function will return false if the file has not been opened in a read state
		bool Read(void* readOutData, size_t sizeOfData, size_t numberOfElements = 1);
		//This function will return false if the file have not been opened in a write state
		bool Write(const void* writeOutData, size_t sizeOfData, size_t numberOfElements = 1);

		bool Seek(int64 sizeOfData, int64 numberOfElements);

		void SeekToStart();
		void SeekToEnd();

		bool IsReading();
		bool IsWriting();

	private:
		bool m_isBinary = false;
		size_t m_fileSize = 0;
		size_t m_currentPosition = 0;
		FILE_OPEN_TYPE m_fileAction = FILE_OPEN_TYPE::NONE;
		void* m_fileHandle = nullptr;

		FileObject m_objectThatStream;
	};


}