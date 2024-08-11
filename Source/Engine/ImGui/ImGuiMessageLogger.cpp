#include "Engine_PCH.h"
#include "ImGuiMessageLogger.h"
#include "imgui.h"
#include "ImGuiHelpers.h"
#include "InternalMessageHandling\InternalMessageLogger.h"

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
    const glm::vec4 localGetColorFromMessageType(eMessageType type)
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

    const char* localGetMessageLogType(eMessageLogType messageType)
    {
        switch (messageType)
        {
        case Hail::eMessageLogType::DrawUniqueueMessages:
            return "Uniqueue messages";
        case Hail::eMessageLogType::DrawAllMessages:
            return "All messages";
        default:
            break;
        }
        return "";
    }
}

Hail::ImGuiMessageLogger::ImGuiMessageLogger()
    : m_currentSortingType(eMessageSortingType::Time)
    , m_logType(eMessageLogType::DrawAllMessages)
{
    m_visibleTypes.Fill(true);
}

void Hail::ImGuiMessageLogger::RenderImGuiCommands()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    ImGui::BeginChild("MessageWindow", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_MenuBar);

    bool updateMessageList = false;

    // Menu Bar:
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("TODO: Visible Message Types"))
        {
            // TODO dropdown
            ImGui::Text("TODO: filter types");
            ImGui::EndMenu();
        }
        if (ImGui::Button("Clear Messages"))
        {
            InternalMessageLogger::GetInstance().ClearMessages();
        }

        if (ImGui::BeginCombo("Display type", localGetMessageLogType(m_logType)))
        {
            for (int n = 0; n < 2; n++)
            {
                const bool is_selected = ((int)(m_logType) == n);
                if (ImGui::Selectable(localGetMessageLogType((eMessageLogType)n), is_selected))
                {
                    m_logType = (eMessageLogType)n;
                    updateMessageList = true;
                }

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::EndMenuBar();
    }

    FillAndSortMessageList(updateMessageList);

    DrawMessageLog();

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

void Hail::ImGuiMessageLogger::DrawMessageLog()
{
    if (m_logType == eMessageLogType::DrawUniqueueMessages)
    {
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
                const InternalMessage& message = *m_sortedMessages[iRow];
                ImGui::TableNextRow(iRow);

                ImGui::TableNextColumn();
                ImGuiHelpers::TextWithHoverHint(localMessageTypeToChar(message.m_type));
                ImGui::TableNextColumn();
                ImGuiHelpers::ColoredTextWithHoverHint(localGetColorFromMessageType(message.m_type), message.m_message);
                ImGui::TableNextColumn();
                ImGui::Text("%i", message.m_numberOfOccurences);
                ImGui::TableNextColumn();
                const uint64 currentTimeInNanoSeconds = (message.m_systemTimeLastHappened);
                FileTime time;
                time.m_highDateTime = currentTimeInNanoSeconds >> 32;
                time.m_lowDateTime = (uint32)currentTimeInNanoSeconds;
                ImGuiHelpers::TextWithHoverHint(ImGuiHelpers::FormattedTimeFromFileData(time).Data());
                ImGui::TableNextColumn();
                ImGuiHelpers::TextWithHoverHint(String256::Format("%s Line : % i", message.m_fileName.Data(), message.m_codeLine));
                ImGui::TableNextColumn();
            }
            ImGui::EndTable();
        }
    }
    else if (m_logType == eMessageLogType::DrawAllMessages)
    {
        const char* topLabels[] = { "Type", "Message", "Occurence Time", "File" };
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
            ImGui::TableHeadersRow();

            for (size_t iRow = 0; iRow < m_sortedMessages.Size(); iRow++)
            {
                H_ASSERT(m_sortedMessages[iRow], "Invalid Message.");
                const InternalMessage& message = *m_sortedMessages[iRow];
                ImGui::TableNextRow(iRow);

                ImGui::TableNextColumn();
                ImGuiHelpers::TextWithHoverHint(localMessageTypeToChar(message.m_type));
                ImGui::TableNextColumn();
                ImGuiHelpers::ColoredTextWithHoverHint(localGetColorFromMessageType(message.m_type), message.m_message);
                ImGui::TableNextColumn();
                const uint64 currentTimeInNanoSeconds = (message.m_systemTimeLastHappened);
                FileTime time;
                time.m_highDateTime = currentTimeInNanoSeconds >> 32;
                time.m_lowDateTime = (uint32)currentTimeInNanoSeconds;
                ImGuiHelpers::TextWithHoverHint(ImGuiHelpers::FormattedTimeFromFileData(time).Data());
                ImGui::TableNextColumn();
                ImGuiHelpers::TextWithHoverHint(String256::Format("%s Line : % i", message.m_fileName.Data(), message.m_codeLine));
                ImGui::TableNextColumn();
            }
            ImGui::EndTable();
        }
    }

}

void Hail::ImGuiMessageLogger::FillAndSortMessageList(bool bUpdateMessageList)
{
    const bool bUpdateList = bUpdateMessageList || InternalMessageLogger::GetInstance().HasRecievedNewMessages();
    if (!bUpdateList)
        return;

    m_sortedMessages.RemoveAll();

    if (m_logType == eMessageLogType::DrawAllMessages)
    {
        const GrowingArray<InternalMessage>& messages = InternalMessageLogger::GetInstance().GetAllMessages();
        const uint32 numberOfMessages = messages.Size();
        for (size_t iMessage = 0; iMessage < numberOfMessages; iMessage++)
        {
            m_sortedMessages.Add(&messages[iMessage]);
        }
    }
    else
    {
        const GrowingArray<InternalMessage>& messages = InternalMessageLogger::GetInstance().GetUniqueueMessages();
        const uint32 numberOfMessages = messages.Size();
        for (size_t iMessage = 0; iMessage < numberOfMessages; iMessage++)
        {
            m_sortedMessages.Add(&messages[iMessage]);
        }
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
