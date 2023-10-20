#pragma once
#include "String.hpp"
#include "Containers\GrowingArray\GrowingArray.h"
#include "Containers\StaticArray\StaticArray.h"
#include "Containers\VectorOnStack\VectorOnStack.h"
#include "Utility\FileSystem.h"

namespace Hail
{
	struct ImGuiFileBrowserData
	{
		GrowingArray<FilePath> objectsToSelect;
		FilePath pathToBeginSearchingIn;
		VectorOnStack<String64, 8> extensionsToSearchFor;
		bool allowMultipleSelection = true;
	};

	class ImGuiFileBrowser
	{
	public:

		bool Init(ImGuiFileBrowserData* dataToBrowseFor);
		void RenderImGuiCommands(bool& unlockApplicationThread);

	private:

		void FileSystemLogic();
		void DirectoryPanelLogic();
		void CopyPath();
		void DeselectFiles();

		bool ValidReturnSelection();
		void AddSelectionToOutput();

		char m_currentDisplayPath[MAX_FILE_LENGTH];
		char m_editedFilePath[MAX_FILE_LENGTH];
		FileSystem m_fileSystem = FileSystem();
		ImGuiFileBrowserData* m_dataToSearchFor = nullptr;
	};
}
