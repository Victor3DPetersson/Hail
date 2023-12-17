#pragma once
#include "Types.h"
namespace Hail
{
	class FileSystem;
	namespace ImGuiHelpers
	{
		//Send in a filesystem to get a Directory panel. 
		bool DirectoryPanelLogic(FileSystem* fileSystem, uint32 minimumDepth);
	}

}