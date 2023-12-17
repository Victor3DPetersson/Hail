#include "Engine_PCH.h"
#include "ImGuiDirectoryPanel.h"


#include "imgui.h"
#include "Utility\FileSystem.h"

using namespace Hail;

bool ImGuiHelpers::DirectoryPanelLogic(FileSystem* fileSystem, uint32 minimumDepth)
{
    uint16_t depthWeReached = 0;
    uint16_t selectedDepth = fileSystem->GetCurrentDepth();
    bool jumpToADepth = false;
    StaticArray<FileObject, MAX_FILE_DEPTH> fileObjects;
    FilePath filePath = fileSystem->GetCurrentFilePath();

    for (size_t i = 0; i < fileSystem->GetCurrentDepth() + 1; i++)
    {
        fileObjects[filePath.GetDirectoryLevel()] = filePath.Object();
        filePath = filePath.Parent();
    }

    FileObject selectedFileObject = fileSystem->GetCurrentFileDirectoryObject();

    for (size_t i = minimumDepth; i < fileSystem->GetCurrentDepth() + 1; i++)
    {
        const FileObject& directoryObject = fileObjects[i];
        String64 folderName = directoryObject.Name().CharString();
        if (i == fileSystem->GetCurrentDepth())
        {
            folderName = "-> " + folderName;
        }
        if (ImGui::Selectable(folderName.Data(), false) && i != fileSystem->GetCurrentDepth())
        {
            if (selectedDepth != directoryObject.GetDirectoryLevel())
                jumpToADepth = true;
            selectedDepth = directoryObject.GetDirectoryLevel();
            selectedFileObject = directoryObject;
        }
        depthWeReached++;
        ImGui::Indent();
        if (i == fileSystem->GetCurrentDepth())
        {
            GrowingArray<SelecteableFileObject>& filesAtDepth = fileSystem->GetFilesAtCurrentDepth();

            for (size_t iFileObject = 0; iFileObject < filesAtDepth.Size(); iFileObject++)
            {
                const SelecteableFileObject& fileObject = filesAtDepth[iFileObject];
                if (!fileObject.m_fileObject.IsDirectory())
                    continue;
                if (ImGui::Selectable(fileObject.m_fileObject.Name().CharString().Data(), fileObject.m_selected))
                {
                    selectedFileObject = fileObject.m_fileObject;
                    selectedDepth = fileObject.m_fileObject.GetDirectoryLevel();
                }
            }
        }
    }

    for (uint32_t i = 0; i < depthWeReached; i++)
    {
        ImGui::Unindent();
    }

    bool fileSystemUpdated = false;
    if (selectedFileObject != fileSystem->GetCurrentFileDirectoryObject() && selectedFileObject.GetDirectoryLevel() >= minimumDepth)
    {
        if (!fileSystem->SetCurrentFileDirectory(selectedFileObject))
        {
            fileSystem->JumpToDepth(selectedDepth);
        }
        fileSystemUpdated = true;
    }
    if (jumpToADepth)
    {
        fileSystem->JumpToDepth(selectedDepth);
        fileSystemUpdated = true;
    }
    return fileSystemUpdated;
}