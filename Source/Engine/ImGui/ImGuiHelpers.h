#pragma once
#include "Types.h"
#include "String.hpp"
#include <Resources\MaterialResources.h>

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
		void MetaResourceTooltipPanel(MetaResource* metaResource);
		String64 FormattedTimeFromFileData(const FileTime& fileTime);

		const char* GetMaterialTypeStringFromEnum(MATERIAL_TYPE type);
		const char* GetMaterialBlendModeFromEnum(BLEND_MODE mode);
	}

}