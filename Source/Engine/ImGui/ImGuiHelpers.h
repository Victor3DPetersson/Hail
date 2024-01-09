#pragma once
#include "Types.h"
#include "String.hpp"

namespace Hail
{
	class MetaResource;
	class FileSystem;
	struct FileTime;

	namespace ImGuiHelpers
	{
		//Send in a filesystem to get a Directory panel. 
		bool DirectoryPanelLogic(FileSystem* fileSystem, uint32 minimumDepth);
		void MetaResourcePanel(MetaResource* metaResource);
		String64 FormattedTimeFromFileData(const FileTime& fileTime);
	}

}