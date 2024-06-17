#include "Engine_PCH.h"
#include "ImGuiMessageLogger.h"
#include "imgui.h"

#include "ErrorHandling\ErrorLogger.h"
#include "ImGuiHelpers.h"
using namespace Hail;

namespace
{
    const char* localMessageTypeToChar(eMessageType type)
    {
        switch (type)
        {
        case Hail::eMessageType::LogMessage:
            return "Message";
        case Hail::eMessageType::Warning:
            return "Warning";
        case Hail::eMessageType::Error:
            return "ERROR";
        case Hail::eMessageType::Fatal:
            return "FATAL";
        default:
            break;
        }
        return "";
    }
    const ImVec4 localGetColorFromMessageType(eMessageType type)
    {
        switch (type)
        {
        case Hail::eMessageType::LogMessage:
            return { 0.9, 0.9, 1.0, 1.0 };
        case Hail::eMessageType::Warning:
            return { 1.0, 0.5, 0.1, 1.0 };
        case Hail::eMessageType::Error:
            return { 1.0, 0.0, 0.0, 1.0 };
        case Hail::eMessageType::Fatal:
            return { 1.0, 0.0, 0.0, 1.0 };
        default:
            break;
        }
        return { 1.0, 1.0, 1.0, 1.0 };
    }
}

Hail::ImGuiMessageLogger::ImGuiMessageLogger() :
    m_currentSortingType(eMessageSortingType::Time)
{
    m_visibleTypes.Fill(true);
}

void Hail::ImGuiMessageLogger::RenderImGuiCommands()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    ImGui::BeginChild("MessageWindow", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_MenuBar);

    // Menu Bar:
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("TODO: Visible Message Types"))
        {
            // TODO dropdown
            ImGui::Text("TODO: filter types");
            ImGui::EndMenu();
        }
        if (ImGui::Button("TODO: Clear Messages"))
        {
        }
        ImGui::EndMenuBar();
    }

    FillAndSortMessageList();

    // Table:
    const char* topLabels[] = { "Type", "Message", "Number of Occurences", "Last Occurence", "File" };
    const int numberOfLabels = (int)eMessageSortingType::Count;
    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;
    tableFlags &= ImGuiTableFlags_BordersOuterV;
    tableFlags &= ImGuiTableFlags_BordersInnerH;
    tableFlags &= ImGuiTableFlags_BordersOuterH;
    tableFlags |= ImGuiTableFlags_BordersInnerV;
    tableFlags |= ImGuiTableFlags_Resizable;
    if (ImGui::BeginTable("Messages", numberOfLabels, tableFlags))
    {
        ImGui::TableSetupColumn(topLabels[0]);
        ImGui::TableSetupColumn(topLabels[1]);
        ImGui::TableSetupColumn(topLabels[2]);
        ImGui::TableSetupColumn(topLabels[3]);
        ImGui::TableSetupColumn(topLabels[4]);
        ImGui::TableHeadersRow();


        for (size_t iRow = 0; iRow < m_sortedMessages.Size(); iRow++)
        {
            H_ASSERT(m_sortedMessages[iRow], "Invalid Message.");
            const ErrorMessage& message = *m_sortedMessages[iRow];
            ImGui::TableNextRow(iRow);

            ImGui::TableNextColumn();
            ImGui::Text(localMessageTypeToChar(message.m_type));
            ImGui::TableNextColumn();
            ImGui::TextColored(localGetColorFromMessageType(message.m_type), message.m_message);
            ImGui::TableNextColumn();
            ImGui::Text("%i", message.m_numberOfOccurences);
            ImGui::TableNextColumn();
            const uint64 currentTimeInNanoSeconds = (message.m_systemTimeLastHappened);
            FileTime time;
            time.m_highDateTime = currentTimeInNanoSeconds >> 32;
            time.m_lowDateTime = (uint32)currentTimeInNanoSeconds;
            ImGui::Text(ImGuiHelpers::FormattedTimeFromFileData(time).Data());
            ImGui::TableNextColumn();
            ImGui::Text("%s Line : % i", message.m_fileName.Data(), message.m_codeLine);
            ImGui::TableNextColumn();
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
}

void Hail::ImGuiMessageLogger::FillAndSortMessageList()
{
    m_sortedMessages.RemoveAll();
    const GrowingArray<ErrorMessage>& messages = ErrorLogger::GetInstance().GetCurrentMessages();
    const uint32 numberOfMessages = messages.Size();
    for (size_t iMessage = 0; iMessage < numberOfMessages; iMessage++)
    {
        m_sortedMessages.Add(&messages[iMessage]);
    }

    // TODO: Sorting
    //for (size_t iMessage = 0; iMessage < numberOfMessages; iMessage++)
    //{
    //    const ErrorMessage* pCurrentMessage = m_sortedMessages[iMessage];
    //    switch (m_currentSortingType)
    //    {
    //    case Hail::ImGuiMessageLogger::eMessageSortingType::MessageType:
    //        break;
    //    case Hail::ImGuiMessageLogger::eMessageSortingType::Message:
    //        break;
    //    case Hail::ImGuiMessageLogger::eMessageSortingType::NumberOfOccurences:
    //        break;
    //    case Hail::ImGuiMessageLogger::eMessageSortingType::Time:
    //        break;
    //    case Hail::ImGuiMessageLogger::eMessageSortingType::Count:
    //        break;
    //    default:
    //        break;
    //    }

    //}
}
