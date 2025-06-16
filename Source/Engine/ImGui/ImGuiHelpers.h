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
		void MetaResourcePanel(const MetaResource* pMetaResource);
		void MetaResourceTooltipPanel(const MetaResource* pMetaResource);
		void TextWithHoverHint(const char* textToShow);
		void ColoredTextWithHoverHint(glm::vec4 color, const char* textToShow);
		String64 FormattedTimeFromFileData(const FileTime& fileTime);

		const char* GetMaterialTypeStringFromEnum(eMaterialType type);
		const char* GetMaterialBlendModeFromEnum(eBlendMode mode);
		const char* GetShaderTypeFromEnum(eShaderStage mode);

		int GetMaterialTypeComboBox(uint32 index);
	}

}