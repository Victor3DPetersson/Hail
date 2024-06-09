#pragma once
#include "Types.h"

namespace Hail
{
	class FilePath;

	struct FileTime
	{
		uint32 m_lowDateTime;
		uint32 m_highDateTime;
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