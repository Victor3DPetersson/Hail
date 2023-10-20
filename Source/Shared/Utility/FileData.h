#pragma once
#include "Types.h"

namespace Hail
{
	class FilePath;

	struct FileTime
	{
		uint32 m_lowDateTime = 0;
		uint32 m_highDateTime = 0;
	};

	struct CommonFileData
	{
		FileTime m_creationTime;
		FileTime m_lastWriteTime;
		uint64 m_filesizeInBytes = 0;
	};

	bool IsValidFilePathInternal(const FilePath* pathToCheck);

	CommonFileData ConstructFileData(void* findData);
	CommonFileData ConstructFileDataFromPath(const FilePath& path);

}