#include "Engine_PCH.h"
#include "ImGuiFileBrowser.h"
#include "imgui.h"
#include "DebugMacros.h"

#include "String.hpp"
#include "MathUtils.h"

#include <locale> 
#include <codecvt>
#include "Utility\FilePath.hpp"
#include <chrono>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

bool Hail::ImGuiFileBrowser::Init(ImGuiFileBrowserData* dataToBrowseFor)
{
    if (m_fileSystem.SetFilePathAndInit(dataToBrowseFor->pathToBeginSearchingIn, dataToBrowseFor->extensionsToSearchFor))
    {
        m_dataToSearchFor = dataToBrowseFor;
        CopyPath();
        return true;
    }
    return false;
}

void Hail::ImGuiFileBrowser::RenderImGuiCommands(bool& unlockApplicationThread)
{
	ImGui::OpenPopup("File Browser");
	if (ImGui::BeginPopupModal("File Browser"))
	{

        if (ImGui::InputText("Adress", m_editedFilePath, IM_ARRAYSIZE(m_currentDisplayPath), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            FilePath newPath = FilePath(m_editedFilePath);
            if (m_fileSystem.SetFilePath(newPath))
            {
                CopyPath();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Up"))
        {
            m_fileSystem.JumpToParent();
            CopyPath();
        }

        ImGui::Separator();

        {
            DirectoryPanelLogic();

            ImGui::SameLine();

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            ImGui::BeginChild("ChildR", ImVec2(0, ImGui::GetContentRegionAvail().y * 0.9f), true, window_flags);
            FileSystemLogic();
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }

        ImGui::Separator();
        ImGui::BeginDisabled();
        bool hasValidSelection = ValidReturnSelection();
        if (hasValidSelection)
        {
            ImGui::EndDisabled();
        }
		if (ImGui::Button("Done"))
		{
            AddSelectionToOutput();
			unlockApplicationThread = true;
		}
        if (!hasValidSelection)
        {
            ImGui::EndDisabled();
        }
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			unlockApplicationThread = true;
		}
		ImGui::EndPopup();
	}
}

void Hail::ImGuiFileBrowser::FileSystemLogic()
{
    const char* topLabels[] = { "Name", "Type", "Size", "DateModified" };
    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;
    tableFlags &= ImGuiTableFlags_BordersOuterV;
    tableFlags &= ImGuiTableFlags_BordersInnerH;
    tableFlags &= ImGuiTableFlags_BordersOuterH;
    tableFlags |= ImGuiTableFlags_BordersInnerV;
    tableFlags |= ImGuiTableFlags_Resizable;

    if (ImGui::BeginTable("Files", 4, tableFlags))
    {
        ImGui::TableSetupColumn(topLabels[0]);
        ImGui::TableSetupColumn(topLabels[1]);
        ImGui::TableSetupColumn(topLabels[2]);
        ImGui::TableSetupColumn(topLabels[3]);
        ImGui::TableHeadersRow();

        GrowingArray<SelecteableFileObject>& files = m_fileSystem.GetFilesAtDepth(m_fileSystem.GetCurrentDepth());
        const uint32_t numberOfFiles = files.Size();
        bool hasUpdatedHierarchy = false;
        SelecteableFileObject selectedFileObject;
        for (int row = 0; row < numberOfFiles; row++)
        {
            ImGui::TableNextRow(row);
            bool iteratingOverFiles = false;
            uint32_t currentObject = row;
            SelecteableFileObject& fileObject = files[currentObject];
            const CommonFileData& fileData = fileObject.m_fileObject.GetFileData();
            ImGui::TableNextColumn();
            String64 fileName;
            wcstombs(fileName, fileObject.m_fileObject.Name(), fileObject.m_fileObject.Length() + 1);
            if (ImGui::Selectable(fileName.Data(), &fileObject.m_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (!ImGui::GetIO().KeyCtrl)    // Clear selection when CTRL is not held
                {
                    DeselectFiles();
                }
                ImGuiIO& imguiIO = ImGui::GetIO();
                if (fileObject.m_fileObject.IsDirectory() && ImGui::IsMouseDoubleClicked(0))
                {
                    selectedFileObject = fileObject;
                    hasUpdatedHierarchy = true;
                }
                else
                {
                    fileObject.m_selected = true;
                }
            }
            ImGui::TableNextColumn();
            char extension[64];
            wcstombs(extension, fileObject.m_fileObject.Extension(), fileObject.m_fileObject.Extension().Length());
            extension[fileObject.m_fileObject.Extension().Length()] = '\0';
            ImGui::Text(extension);
            ImGui::TableNextColumn();
            ImGui::Text("%f mb", static_cast<float>((static_cast<double>(fileData.m_filesizeInBytes) / 1000000.f)));

            String64 timePreview;
#ifdef PLATFORM_WINDOWS
            FILETIME time;
            time.dwLowDateTime = fileData.m_lastWriteTime.m_lowDateTime;
            time.dwHighDateTime = fileData.m_lastWriteTime.m_highDateTime;
            SYSTEMTIME SystemTime{};
            FileTimeToSystemTime(&time, &SystemTime);
            timePreview = String64::Format("%u/%u/%u  %u:%u:%u", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
#endif
            ImGui::TableNextColumn();
            ImGui::Text(timePreview.Data());
        }
        ImGui::EndTable();

        if (hasUpdatedHierarchy)
        {
            m_fileSystem.JumpUpOneDirectory(selectedFileObject);
            CopyPath();
        }
    }
}

void Hail::ImGuiFileBrowser::DirectoryPanelLogic()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
    ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.2f, ImGui::GetContentRegionAvail().y * 0.9f), false, window_flags);
    uint16_t depthWeReached = 0;
    uint16_t selectedDepth = 0;
    bool jumpToADepth = false;
    for (uint16_t i = 0; i < m_fileSystem.GetMaxDepth(); i++)
    {
        const auto& folder = m_fileSystem.GetDirectoryAtDepth(i);
        String64 folderName;
        wcstombs(folderName, folder.m_fileObject.Name(), folder.m_fileObject.Length() + 1);
        if (i == m_fileSystem.GetCurrentDepth() - 1)
        {
            folderName = "-> " + folderName;
        }
        if (ImGui::Selectable(folderName.Data(), folder.m_selected) && i != m_fileSystem.GetCurrentDepth() - 1)
        {
            jumpToADepth = true;
            selectedDepth = folder.m_fileObject.GetDirectoryLevel();
        }
        depthWeReached++;
        ImGui::Indent();
    }
    for (uint32_t i = 0; i < depthWeReached; i++)
    {
        ImGui::Unindent();
    }

    ImGui::EndChild();

    if (jumpToADepth)
    {
        m_fileSystem.JumpToDepth(selectedDepth);
        CopyPath();
    }
}

void Hail::ImGuiFileBrowser::CopyPath()
{
    const FilePath& convertedPath = m_fileSystem.GetCurrentFilePath();
    wcstombs(m_currentDisplayPath, convertedPath.Data(), wcslen(convertedPath.Data()) + 1);
    uint32_t length = strlen(m_currentDisplayPath);
    memcpy(m_editedFilePath, m_currentDisplayPath, length * sizeof(char));
    m_editedFilePath[length] = '\0';
}

void Hail::ImGuiFileBrowser::DeselectFiles()
{
    GrowingArray<SelecteableFileObject>& files = m_fileSystem.GetFilesAtDepth(m_fileSystem.GetCurrentDepth());
    const uint32_t numberOfFiles = files.Size();
    for (int object = 0; object < numberOfFiles; object++)
    {
        files[object].m_selected = false;
    }
}

bool Hail::ImGuiFileBrowser::ValidReturnSelection()
{
    const GrowingArray<SelecteableFileObject>& files = m_fileSystem.GetFilesAtDepth(m_fileSystem.GetCurrentDepth());
    if (files.Size())
    {
        for (uint32_t i = 0; i < files.Size(); i++)
        {
            if (files[i].m_selected && !files[i].m_fileObject.IsDirectory())
            {
                return true;
            }
        }
    }
    return false;
}

void Hail::ImGuiFileBrowser::AddSelectionToOutput()
{
    const GrowingArray<SelecteableFileObject>& files = m_fileSystem.GetFilesAtDepth(m_fileSystem.GetCurrentDepth());
    if (files.Size())
    {
        for (uint32_t i = 0; i < files.Size(); i++)
        {
            if (files[i].m_selected && !files[i].m_fileObject.IsDirectory())
            {
                FilePath pathToAdd = m_fileSystem.GetCurrentFilePath();
                pathToAdd = pathToAdd + files[i].m_fileObject;
                m_dataToSearchFor->objectsToSelect.Add(pathToAdd);
            }
        }
    }

}
